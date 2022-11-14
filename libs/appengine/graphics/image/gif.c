/* --------------------------------------------------------------------------
 *    Name: gif.c
 * Purpose: GIF module
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/draw.h"
#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/base/bitwise.h"
#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"
#include "appengine/io/filing.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"

#include "bitmap.h"
#include "appengine/graphics/image.h"

#include "gif.h"

enum
{
  MAX_LZW_BITS  = 12,
  MAX_LZW_CODES = ((1 << MAX_LZW_BITS) - 1),

  IO_ERROR          = -1,
  BAD_CODE_SIZE     = -2,
  OUT_OF_MEMORY     = -3,
  CODE_OUT_OF_RANGE = -4
};

/* #pragma -z0  optimisation (was) cock(ing) up */

static const struct
{
  int starts[4];
  int offsets[4];
}
SC =
{
  { 0, 4, 2, 1 },
  { 8, 8, 4, 2 }
};

typedef struct
{
  /* sprite construction */

  byte *sptr;

  unsigned int *global_palette;
  unsigned int *local_palette;
  unsigned int *palette;

  int global_width;
  int global_height;
  int global_bpp;

  int local_width;
  int local_height;
  int local_bpp;

  int sprrowbytes;
  int sprimgbytes;
  int bpp;
  int pass;
  int y;  /* for de-interlacing */

  osbool is_interlaced;


  /* decoder */

  /* Static variables */
  int curr_size;               /* The current code size */
  int clear;                   /* Value for a clear code */
  int ending;                  /* Value for a ending code */
  int newcodes;                /* First available code */
  int top_slot;                /* Highest code for current size */
  int slot;                    /* Last read code */

  /* The following static variables are used for separating out codes */
  int bytes_left;                  /* # bytes left in block */
  int bits_left;               /* # bits left in current byte */

  char *stack;                 /* Stack for storing pixels */
  char *suffix;                /* Suffix table */
  unsigned int *prefix;        /* Prefix linked list */


  /* getnextcode */

  char b; /* Current byte */
  char block[257]; /* Current block */ /* XXX static block */
  char *pbytes; /* Pointer to next byte in block */
}
State;

static int giflzw_decoder(State *S, int linewidth);

static void tidyup(State *S)
{
#ifndef NDEBUG
  fprintf(stderr, "tidyup\n");
#endif

  buffer_stop();

  if (S->global_palette != NULL)
  {
    free(S->global_palette);
    S->global_palette = NULL;
  }
  if (S->local_palette != NULL)
  {
    free(S->local_palette);
    S->local_palette = NULL;
  }

  xhourglass_off();
}

static unsigned int *gif_load_palette(int entries)
{
  int realentries;
  unsigned int *palette;
  unsigned int *p;
  int i;
  int r, g, b;
  unsigned int entry;

  if (entries == 8)
    realentries = 16;
  else if (entries >= 32)
    realentries = 256;
  else
    realentries = entries;

#ifndef NDEBUG
  fprintf(stderr, "gif_load_palette: about to claim palette\n");
#endif
  palette = malloc(realentries * 4 * 2);
  if (palette != NULL)
  {
    p = palette;

    for (i = 0; i < entries; i++)
    {
      if ((r = buffer_getbyte()) < 0 || (g = buffer_getbyte()) < 0 ||
          (b = buffer_getbyte()) < 0)
      {
        free(palette);
        return NULL;
      }
      entry = b << 24 | g << 16 | r << 8;
      *p++ = entry;  /* first flash state */
      *p++ = entry;  /* second flash state */
    }

    /* Fill in the rest of the palette with a grey scale. */
    for ( /* i = entries */ ; i < realentries; i++)
    {
      entry = i * 0x01010100;
      *p++ = entry;
      *p++ = entry;
    }
  }

  return palette;
}

static result_t gif_load(const image_choices *choices, image_t *image)
{
  State S;
  int log2bpp;
  int c;
  int d;
  int blocksize;
  int sprpalbytes;
  char buffer[257]; /* local image descriptor (plus one byte for string
    terminator) */
  osspriteop_area *area;
  osspriteop_header *header;
  os_mode mode;

  NOT_USED(choices);

  xhourglass_on();

  S.global_palette = NULL;
  S.local_palette  = NULL;

#ifndef NDEBUG
  fprintf(stderr, "gif_load\n");
#endif
  osfile_read_no_path(image->file_name,
                     &image->source.load,
                     &image->source.exec,
             (int *) &image->source.file_size,
                      NULL);

#ifndef NDEBUG
  fprintf(stderr, "gif_load: about to call buffer_start\n");
#endif
  if (buffer_start(image->file_name, 16384 /* buffer size */) < 0)
  {
    tidyup(&S);
    return oserror_build(0, "error.buffer.start"); /* XXX poor (fixed) error */
  }

#ifndef NDEBUG
  fprintf(stderr, "Checking header\n");
#endif
  buffer_getblock(buffer, 6); /* XXX error check */
  if (buffer[0] != 'G' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3]
    != '8' || (buffer[4] != '7' && buffer[4] != '9') || buffer[5] != 'a')
  {
    tidyup(&S);
    return oserror_build(0, "error.gif.not");
  }

#ifndef NDEBUG
  fprintf(stderr, ": Global descriptor\n");
#endif
 /*
  * <Packed Fields> = Global Color Table Flag  1 bit
  * Color Resolution                           3 bits
  * Sort Flag                                  1 bit
  * Size of Global Color Table                 3 bits
  */

  buffer_getblock(buffer, 7);
  /* XXX error check */
  S.global_width  = buffer[0] + buffer[1] * 256;
  S.global_height = buffer[2] + buffer[3] * 256;
#ifndef NDEBUG
  fprintf(stderr, " w=%d h=%d flags=%x bgindex=%x aspect=%x\n", S.global_width,
          S.global_height, buffer[4], buffer[5], buffer[6]);
#endif
  if (buffer[4] & 0x80)
  {
    /* has global palette */
    S.global_bpp = (buffer[4] & 0x07) + 1;
    S.global_palette = gif_load_palette(1 << S.global_bpp);
    if (S.global_palette == NULL)
    {
#ifndef NDEBUG
      fprintf(stderr, "failed to load global palette\n");
#endif
      tidyup(&S);
      return result_OOM; /* XXX or bad.gif */
    }
  }

#ifndef NDEBUG
  fprintf(stderr, "Entering header decoding loop\n");
#endif
  for (;;)
  {
    c = buffer_getbyte();
    if (c == ';')
    {
#ifndef NDEBUG
      fprintf(stderr, ": Terminator\n");
#endif
      /* terminator */
      continue;
    }
    else if (c == 0x21 /* Extension */ )
    {
#ifndef NDEBUG
      fprintf(stderr, ": Extension\n");
#endif
      d = buffer_getbyte();
      if (d == 0xff) /* Application Extension */
      {
#ifndef NDEBUG
        fprintf(stderr, "> Application\n");
#endif
        for (blocksize = buffer_getbyte(); blocksize; blocksize =
               buffer_getbyte())
        {
          buffer_getblock(buffer, blocksize);
          buffer[blocksize] = '\0';
          if (strncmp((char *) buffer, "NETSCAPE2.0", 11) == 0)
          {
            /* Netscape 2.0 extension - fetch the next sub-block */
            blocksize = buffer_getbyte(); /* block size */
            buffer_getblock(buffer, blocksize);
#ifndef NDEBUG
            fprintf(stderr, "Netscape 2.0, %d, %d\n", buffer[1], buffer[2] +
                    buffer[3] * 256);
#endif
          }
        }
      }
      else if (d == 0xf9) /* Graphic Control Extension */
      {
#ifndef NDEBUG
        fprintf(stderr, "> Graphic Control\n");
#endif
        blocksize = buffer_getbyte(); /* block size */
        buffer_getblock(buffer, blocksize);      /* should be 4 bytes */
#ifndef NDEBUG
        if (blocksize != 4)
          fprintf(stderr, " graphic control block size is not 4!\n");
        if (buffer[0] & 1 /*  */)
          fprintf(stderr, " transparent index given (%d.)\n", buffer[3]);
        fprintf(stderr, " delay %d\n", buffer[1] + buffer[2] * 256);
#endif
        blocksize = buffer_getbyte(); /* block size (zero) */
      }
      /* treat comment extensions as unknown until we do something useful
       * with them */
#if 0
      else if (d == 0xfe) /* Comment Extension */
      {
#ifndef NDEBUG
        fprintf(stderr, "> Comment\n");
#endif
        for (blocksize = buffer_getbyte(); blocksize; blocksize =
               buffer_getbyte())
        {
          buffer_getblock(buffer, blocksize);
#ifndef NDEBUG
          buffer[blocksize] = '\0';
          fprintf(stderr, "Comment: %s\n", buffer);
#endif
        }
      }
#endif
      else
      {
#ifndef NDEBUG
        fprintf(stderr, "> Unrecognised extension '%02x'\n", d);
#endif
        for (blocksize = buffer_getbyte(); blocksize; blocksize =
               buffer_getbyte())
          buffer_getblock(buffer, blocksize);
      }
      continue;
    }
    else if (c == ',')
    {
#ifndef NDEBUG
      fprintf(stderr, ": Local descriptor\n");
#endif
      /* local image descriptor */
      buffer_getblock(buffer, 9);
      /* XXX error check */
      S.local_width  = buffer[4] + buffer[5] * 256;
      S.local_height = buffer[6] + buffer[7] * 256;
#ifndef NDEBUG
      fprintf(stderr, " w=%d h=%d flags=%x bgindex=%x aspect=%x\n", S.local_width,
        S.local_height, buffer[8], buffer[9], buffer[10]);
#endif
      S.is_interlaced = (buffer[8] & 0x40) != 0;
      if (buffer[8] & 0x80)
      {
        /* local colourmap present */
        S.local_bpp = (buffer[8] & 0x07) + 1;
        S.local_palette = gif_load_palette(1 << S.local_bpp);
        if (S.local_palette == NULL)
        {
#ifndef NDEBUG
          fprintf(stderr, "failed to load local palette\n");
#endif
          tidyup(&S);
          return result_OOM;
        }

        /* we have a local palette */
        S.bpp = (S.local_bpp == 3) ? 4 :
                (S.local_bpp  > 4) ? 8 :
                                     S.local_bpp;

        S.palette = S.local_palette;
      }
      else
      {
        /* local colourmap not present */
        if (S.global_palette == NULL)
        {
          /* no palette was specified - invent an 8bpp global one (which
           * will be used for subsequent unpaletted images) */
          S.global_bpp = S.bpp = 8;
#ifndef NDEBUG
          fprintf(stderr, "gif_load: about to claim global_palette\n");
#endif
          S.global_palette = malloc(256 * 4 * 2);
          if (S.global_palette == NULL)
          {
#ifndef NDEBUG
            fprintf(stderr, "failed to alloc space for emergency global palette\n");
#endif
            tidyup(&S);
            return result_OOM;
          }

          make_grey_palette(3, S.global_palette);
        }
        else
        {
          if (S.global_bpp == 3)
            S.bpp = 4;
          else if (S.global_bpp > 4)
            S.bpp = 8;
          else
            S.bpp = S.global_bpp;
        }

        S.palette = S.global_palette;
      }
      break;
    }
    else
    {
#ifndef NDEBUG
      fprintf(stderr, "Unknown thingy\n");
#endif
      /* no idea */
      continue;
    }
  }

  /* At this point the next byte in the stream is the code size. */

#ifndef NDEBUG
  fprintf(stderr, "Creating sprite\n");
#endif
  S.sprrowbytes = ((S.local_width * S.bpp + 31) & ~31) >> 3;
  sprpalbytes = 1 << (S.bpp + 3);
  S.sprimgbytes = 16 + 44 + sprpalbytes + S.sprrowbytes * S.local_height;

  if (flex_alloc((flex_ptr) &area, S.sprimgbytes) == 0)
  {
#ifndef NDEBUG
    fprintf(stderr, "failed to alloc space for sprite\n");
#endif
    tidyup(&S);
    return result_OOM;
  }

#ifndef NDEBUG
  fprintf(stderr, "output sprite is at %p and is %d bytes long %p\n",
          (void *) area, S.sprimgbytes, (char *) area + S.sprimgbytes);
#endif

  area->size = S.sprimgbytes;
  area->first = 16;
  osspriteop_clear_sprites(osspriteop_USER_AREA, area);

  mode = sprite_mode(1, 1, (int) floorlog2(S.bpp), FALSE); /* 90x90xlog2bpp */

  read_mode_vars(mode, &image->display.dims.bm.xeig,
                       &image->display.dims.bm.yeig,
                       &log2bpp);

  osspriteop_create_sprite(osspriteop_NAME, area, "gif", 0 /* no
    palette */, S.local_width, S.local_height, mode);

  header = sprite_select(area, 0);

  osspriteop_create_true_palette(osspriteop_PTR, area,
                                 (osspriteop_id) header);

  memcpy((char *) header + 44, (char *) S.palette, sprpalbytes);

  S.sptr = (byte *) header + 44 + sprpalbytes;

#ifndef NDEBUG
  fprintf(stderr, "Starting decode\n");
#endif
  if (S.is_interlaced)
  {
    S.pass = 0;
    S.y = SC.starts[S.pass];
  }
  else
  {
    S.y = 0;
  }

  {
#ifndef NDEBUG
    os_t start, stop;
#endif
    const char *errtok;

#ifndef NDEBUG
    start = os_read_monotonic_time();
#endif
    switch (giflzw_decoder(&S, S.local_width))
    {
    case IO_ERROR:          errtok = "error.gif.ioerror";        break;
    case BAD_CODE_SIZE:     errtok = "error.gif.badcodesize";    break;
    case OUT_OF_MEMORY:     errtok = "error.gif.outofmem";       break;
    case CODE_OUT_OF_RANGE: errtok = "error.gif.codeoutofrange"; break;
    default:                errtok = NULL;                       break;
    }

    if (errtok)
      oserror_report(0, errtok);
#ifndef NDEBUG
    stop = os_read_monotonic_time();
    fprintf(stderr, "Time: %d\n", stop - start);
#endif
  }

#ifndef NDEBUG
  fprintf(stderr, "Finished decode\n");
#endif

  image->flags = image_FLAG_COLOUR | image_FLAG_CAN_ROT;

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &area);

  image->display.dims.bm.width  = S.local_width;
  image->display.dims.bm.height = S.local_height;
  /* xeig, yeig set */
  image->display.dims.bm.xdpi   = os_INCH >> image->display.dims.bm.xeig;
  image->display.dims.bm.ydpi   = os_INCH >> image->display.dims.bm.yeig;
  image->display.dims.bm.bpp    = 1 << log2bpp;
  image->display.file_size      = S.sprimgbytes;
  image->display.file_type      = osfile_TYPE_SPRITE;

  image->source.dims = image->display.dims;
  /* load, exec, file_size set */
  image->source.file_type       = osfile_TYPE_SPRITE;

  image->scale.min = 1;
  image->scale.max = 32767;

  image->details.sprite.mode = mode;

  tidyup(&S);

#ifndef NDEBUG
  fprintf(stderr, "Done\n\n");
#endif
  return result_OK;
}

/* DECODE.C - An LZW decoder for GIF
 * Copyright (C) 1987, by Steven A. Bennett */

/* This function takes a full line of pixels (one byte per pixel) and
 * displays them (or does whatever your program wants with them...).  It
 * should return zero, or negative if an error or some other event occurs
 * which would require aborting the decode process...  Note that the length
 * passed will almost always be equal to the line length passed to the
 * decoder function, with the sole exception occurring when an ending code
 * occurs in an odd place in the GIF file...  In any case, linelen will be
 * equal to the number of pixels passed...
 */

static void giflzw_doneline(State *S, const char *pixels, int linelen)
{
  int old_y;
  int shift;
  unsigned int pixel;
  unsigned int *sprite_pixels;

  if (S->y >= S->local_height)
    return;

  /* Condense down the raw output into a row of "packed" sprite data. */
  sprite_pixels = (unsigned int *) S->sptr;
  while (linelen > 0)
  {
    shift = 0;
    pixel = 0;
    while (shift < 32 && linelen > 0)
    {
      pixel |= *pixels++ << shift;
      shift += S->bpp;
      linelen--;
    }
    *sprite_pixels++ = pixel;
  }

  /* Set up the next sprite row pointer */
  if (S->is_interlaced)
  {
    old_y = S->y;
    S->y += SC.offsets[S->pass];
    if (S->y >= S->local_height)
      S->y = SC.starts[++S->pass];
    S->sptr += (S->y - old_y) * S->sprrowbytes;
  }
  else
  {
    S->y++;
    S->sptr += S->sprrowbytes;
  }
}

/* giflzw_getnextcode()
 * - gets the next code from the GIF file.  Returns the code, or else
 * a negative number in case of file errors... */
static unsigned int giflzw_getnextcode(State *S)
{
  static const unsigned int mask[16] =
  {
    0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f,
    0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff
  };

  int status;
  unsigned int ret;

  if (S->bits_left == 0)
  {
    if (S->bytes_left <= 0)
    {
      /* Out of bytes in current block, so read next block */
      S->pbytes = S->block;
      S->bytes_left = buffer_getbyte();
      if (S->bytes_left < 0)
        return S->bytes_left;

      if (S->bytes_left)
      {
        status = buffer_getblock((char *) S->block, S->bytes_left);
        if (status <= 0)
          return status;
      }
    }
    S->b = *S->pbytes++;
    S->bits_left = 8;
    S->bytes_left--;
  }
  ret = S->b >> (8 - S->bits_left);
  while (S->curr_size > S->bits_left)
  {
    if (S->bytes_left <= 0)
    {
      /* Out of bytes in current block, so read next block */
      S->pbytes = S->block;
      S->bytes_left = buffer_getbyte();
      if (S->bytes_left < 0)
        return S->bytes_left;

      if (S->bytes_left)
      {
        status = buffer_getblock((char *) S->block, S->bytes_left);
        if (status <= 0)
          return status;
      }
    }
    S->b = *S->pbytes++;
    ret |= S->b << S->bits_left;
    S->bits_left += 8;
    S->bytes_left--;
  }

  S->bits_left -= S->curr_size;
  return ret & mask[S->curr_size];
}

/* int giflzw_decoder(linewidth)
 *    int linewidth;                 * Pixels per line of image *
 *
 * - This function decodes an LZW image, according to the method used
 * in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
 * will generate a call to giflzw_doneline(), which is a user specific
 * function to display a line of pixels.  The function gets it's codes from
 * giflzw_getnextcode() which is responsible for reading blocks of data and
 * separating them into the proper size codes.  Finally,
 * giflzw_buffer_getbyte() is the global routine to read the next byte from
 * the GIF file.
 *
 * It is generally a good idea to have linewidth correspond to the actual
 * width of a line (as specified in the Image header) to make your own
 * code a bit simpler, but it isn't absolutely necessary.
 *
 * Returns: 0 if successful, else negative.  (See ERRS.H)
 */

static int giflzw_decoder(State *S, int linewidth)
{
  int   size;

  char *buf;

  int   oc;
  int   fc;

  char *sp;
  char *bufptr;
  int   bufcnt;

  int   code;

  int   ret;

  /* Initialize for decoding a new image... */
  size = buffer_getbyte();
  if (size < 0)
    return size;

  if (size < 1 || size > 9)
    return BAD_CODE_SIZE;

  if (size == 1)
    size++;

  S->curr_size = size + 1;
  S->clear     = 1 << size;
  S->ending    = S->clear + 1;
  S->slot      =
  S->newcodes  = S->ending + 1;
  S->top_slot  = 1 << S->curr_size;

  S->bytes_left = 0;
  S->bits_left  = 0;

#ifndef NDEBUG
  fprintf(stderr, "giflzw_decoder: about to claim stack\n");
#endif
  S->stack = malloc((MAX_LZW_CODES + 1) * sizeof(char) +
                    (MAX_LZW_CODES + 1) * sizeof(char) +
                    (MAX_LZW_CODES + 1) * sizeof(unsigned int) +
                    (linewidth + 1) * sizeof(char));
  if (S->stack == NULL)
    return OUT_OF_MEMORY;

  S->suffix = S->stack + ((MAX_LZW_CODES + 1) * sizeof(char));
  S->prefix = (unsigned int *) (S->suffix + ((MAX_LZW_CODES + 1) * sizeof(char)));
  buf = ((char *) S->prefix) + (MAX_LZW_CODES + 1) * sizeof(unsigned int);

  /* Initialize in case they forgot to put in a clear code.
   * (This shouldn't happen, but we'll try and decode it anyway...) */
  oc = fc = 0;

  /* Set up the stack pointer and decode buffer pointer */
  sp     = S->stack;
  bufptr = buf;
  bufcnt = linewidth;

 /*
  * This is the main loop.  For each code we get we pass through the
  * linked list of prefix codes, pushing the corresponding "character" for
  * each code onto the stack.  When the list reaches a single "character"
  * we push that on the stack too, and then start unstacking each
  * character for output in the correct order.  Special handling is
  * included for the clear code, and the whole thing ends when we get
  * an ending code.
  */

  while ((code = giflzw_getnextcode(S)) != S->ending)
  {

    /* If we had a file error, return without completing the decode */
    if (code < 0)
    {
      free(S->stack);
      return code;
    }

    /* If the code is a clear code, reinitialize all necessary items.*/
    if (code == S->clear)
    {
      S->curr_size = size + 1;
      S->slot      = S->newcodes;
      S->top_slot  = 1 << S->curr_size;

     /*
      * Continue reading codes until we get a non-clear code
      * (Another unlikely, but possible case...)
      */

      while ((code = giflzw_getnextcode(S)) == S->clear)
        ;

     /*
      * If we get an ending code immediately after a clear code
      * (Yet another unlikely case), then break out of the loop.
      */

      if (code == S->ending)
        break;

     /*
      * Finally, if the code is beyond the range of already set codes,
      * (This one had better NOT happen...  I have no idea what will
      * result from this, but I doubt it will look good...) then set it
      * to color zero.
      */

      if (code >= S->slot)
        code = 0;

      oc = fc = code;

     /*
      * And let us not forget to put the char into the buffer... And
      * if, on the off chance, we were exactly one pixel from the end
      * of the line, we have to send the buffer to the giflzw_doneline()
      * routine...
      */

      *bufptr++ = code;
      if (--bufcnt == 0)
      {
        giflzw_doneline(S, buf, linewidth);
        bufptr = buf;
        bufcnt = linewidth;
      }

    }
    else
    {
      int code2;

     /*
      * In this case, it's not a clear code or an ending code, so
      * it must be a code code...  So we can now decode the code into
      * a stack of character codes. (Clear as mud, right?)
      */

      code2 = code;

     /*
      * Here we go again with one of those off chances.  If, on the
      * off chance, the code we got is beyond the range of those already
      * set up (Another thing which had better NOT happen...) we trick
      * the decoder into thinking it actually got the last code read.
      * (Hmmn... I'm not sure why this works..  But it does..)
      */

      if (code2 >= S->slot)
      {
        if (code2 > S->slot)
          return CODE_OUT_OF_RANGE;
        *sp++ = fc;
        code2 = oc;
      }

     /*
      * Here we scan back along the linked list of prefixes, pushing
      * helpless characters (ie. suffixes) onto the stack as we do so.
      */

      while (code2 >= S->newcodes)
      {
        *sp++ = S->suffix[code2];
        code2 = S->prefix[code2];
      }

     /*
      * Push the last character on the stack, and set up the new
      * prefix and suffix, and if the required slot number is greater
      * than that allowed by the current bit size, increase the bit
      * size.  (NOTE - If we are all full, we *don't* save the new
      * suffix and prefix...  I'm not certain if this is correct...
      * it might be more proper to overwrite the last code...
      */

      *sp++ = code2;
      if (S->slot < S->top_slot)
      {
        fc = code2;
        S->suffix[S->slot] = fc;
        S->prefix[S->slot] = oc;
        S->slot++;
        oc = code;
      }

      if (S->slot >= S->top_slot)
      {
        if (S->curr_size < MAX_LZW_BITS)
        {
          S->top_slot <<= 1;
          S->curr_size++;
        }
      }

     /*
      * Now that we've pushed the decoded string (in reverse order)
      * onto the stack, lets pop it off and put it into our decode
      * buffer...  And when the decode buffer is full, write another
      * line...
      */

      while (sp > S->stack)
      {
        *bufptr++ = *(--sp);
        if (--bufcnt == 0)
        {
          giflzw_doneline(S, buf, linewidth);
          bufptr = buf;
          bufcnt = linewidth;
        }
      }
    }
  }
  ret = 0;

#ifndef NDEBUG
 /*
  * If there's things left in the buffer on exit... stuff 'em. Otherwise this
  * would be emitting data into an already full image. (I think) [This needs
  * to be verified.]
  */

  if (bufcnt != linewidth)
    fprintf(stderr, "%d bytes left\n", linewidth - bufcnt);
    /*giflzw_doneline(buf, linewidth - bufcnt);*/
#endif

  free(S->stack);

  return ret;
}

void gif_export_methods(const image_choices *choices, image_t *image)
{
  static const image_methods methods =
  {
    gif_load,
    bitmap_save,
    bitmap_unload,
    bitmap_histogram,
    bitmap_rotate,
    NULL,
    NULL
  };

  NOT_USED(choices);

  image->methods = methods;
}
