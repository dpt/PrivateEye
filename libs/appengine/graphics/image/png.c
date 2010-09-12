/* --------------------------------------------------------------------------
 *    Name: png.c
 * Purpose: PNG module
 * Version: $Id: png.c,v 1.6 2010-01-06 00:36:20 dpt Exp $
 * ----------------------------------------------------------------------- */

  // need to be able to report aspect ratio w/o phys size
  // want "Xbpp colour + Ybpp alpha"

#include "kernel.h"
#include "swis.h"

#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/colourtrans.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/base/bitwise.h"
#include "appengine/base/oserror.h"
#include "appengine/base/messages.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"

#include "png/png.h"

#include "bitmap.h"
#include "appengine/graphics/image.h"

#include "png.h"

static void png_read_row_callback(png_structp png_ptr, png_uint_32 row_number, int pass)
{
  NOT_USED(pass);

  /* XXX How do I get the height and total number of passes, legally, so that
   *     I can correctly work out the percentage?  */
  hourglass_percentage((int) row_number * 100 / (int) png_ptr->height);
}

static void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
  NOT_USED(png_ptr);

  oserror__report(0, "error.png.error", error_msg);
}

static void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
  NOT_USED(png_ptr);

  oserror__report(0, "error.png.warning", warning_msg);
}

static int png_load(image_choices *choices, image *image)
{
  int                source_xdpi, source_ydpi;
  int                source_bpp;
  bits               source_load;
  bits               source_exec;
  int                source_file_size;
  FILE              *fp = NULL;
  byte              *sprite_ptr;
  int                bit_depth;
  int                colour_type;
  int                i;
  int                log2bpp;
  int                num_palette;
  int                sprite_imgbytes;
  int                sprite_palbytes;
  int                sprite_rowbytes;
  int                sprite_bpp;
  osspriteop_area   *area;
  osspriteop_header *header;
  os_mode            mode;
  png_colorp         palette;
  png_infop          info_ptr;
  png_structp        png_ptr = NULL;
  png_uint_32        height;
  png_uint_32        width;
  png_uint_32        h;
  png_bytep         *row_pointers;
  osbool             has_alpha = FALSE;
  osbool             spr_mask  = FALSE;
  png_bytep          trans;
  int                num_trans;

  NOT_USED(choices);

  hourglass_on();

  osfile_read_no_path(image->file_name,
                     &source_load,
                     &source_exec,
                     &source_file_size,
                      NULL /* attr */);

  fp = fopen(image->file_name, "rb");
  if (fp == NULL)
  {
    oserror__report(0, "error.fopen");
    goto CleanUp; /* failure */
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                       (png_voidp) NULL,
                   (png_error_ptr) user_error_fn,
                   (png_error_ptr) user_warning_fn);
  if (png_ptr == NULL)
  {
    oserror__report(0, "error.png.create");
    goto CleanUp; /* failure */
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
  {
    oserror__report(0, "error.png.info");
    goto CleanUp; /* failure */
  }

  if (setjmp(png_ptr->jmpbuf))
    goto CleanUp; /* failure */

  png_init_io(png_ptr, fp);

  png_set_read_status_fn(png_ptr, png_read_row_callback);

  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
               &colour_type, NULL, NULL, NULL);
#ifndef NDEBUG
  fprintf(stderr, "png: w=%d h=%d bit_depth=%d colour_type=%d\n",
          (int) width, (int) height, bit_depth, colour_type);
#endif

  source_bpp = bit_depth;
  switch (colour_type)
  {
    case PNG_COLOR_TYPE_PALETTE:                     break;
    case PNG_COLOR_TYPE_GRAY:                        break;
    case PNG_COLOR_TYPE_GRAY_ALPHA: source_bpp *= 2; break;
    case PNG_COLOR_TYPE_RGB:        source_bpp *= 3; break;
    case PNG_COLOR_TYPE_RGB_ALPHA:  source_bpp *= 4; break;
  }

  /* discard the lower bytes of any 16-bit-per-channel images */
  if (bit_depth == 16)
  {
#ifndef NDEBUG
    fprintf(stderr, "png: 16bpc -> 8bpc\n");
#endif
    png_set_strip_16(png_ptr);
    bit_depth = 8;
  }

  switch (colour_type)
  {
    case PNG_COLOR_TYPE_PALETTE:
#ifndef NDEBUG
      fprintf(stderr, "png: colour type: palette\n");
#endif
      /* bit depths 1, 2, 4, 8, (16) */

      /* change the order of packed pixels to LSB first */
      if (bit_depth < 8)
        png_set_packswap(png_ptr);

      sprite_bpp = bit_depth;
      break;

    case PNG_COLOR_TYPE_GRAY:
#ifndef NDEBUG
      fprintf(stderr, "png: colour type: grey\n");
#endif
      /* bit depths 1, 2, 4, 8, (16) */

      /* change the order of packed pixels to LSB first */
      if (bit_depth < 8)
        png_set_packswap(png_ptr);

      sprite_bpp = bit_depth;
      break;

    case PNG_COLOR_TYPE_GRAY_ALPHA:
#ifndef NDEBUG
      fprintf(stderr, "png: colour type: grey+alpha\n");
#endif
      /* bit depths 8, (16) */
      /* grey + alpha, output RGBA quads */
      png_set_gray_to_rgb(png_ptr);
      has_alpha = TRUE;
      sprite_bpp = 32;
      break;

    case PNG_COLOR_TYPE_RGB:
#ifndef NDEBUG
      fprintf(stderr, "png: colour type: rgb\n");
#endif
      /* bit depths 8, (16) */
      /* RGBA, output RGBA quads */
      png_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
      sprite_bpp = 32;
      break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
#ifndef NDEBUG
      fprintf(stderr, "png: colour type: rgb+alpha\n");
#endif
      /* bit depths 8, (16) */
      /* RGBA, output RGBA quads */
      has_alpha = TRUE;
      sprite_bpp = 32;
      break;

    default:
      /* unknown colour type */
      oserror__report(0, "error.png.colour");
      goto CleanUp; /* failure */
  }

  /* Expand tRNS (transparent palette entries or single transparent colour
   * in RGB) to full RGBA quads */
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
  {
#if 1
    if (colour_type == PNG_COLOR_TYPE_PALETTE ||
        colour_type == PNG_COLOR_TYPE_GRAY)
        // does PNG_COLOR_TYPE_GRAY have this case?
    {
      // see if we should use a mask rather than alpha

      // NULL for trans_values since we're only picking up
      // paletted stuff

      png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, NULL);

      if (num_trans == 1)
      {
        // is the trans value zero?
        if (trans[0] == 0)
          // completely transparent - can use paletted image + mask
          spr_mask = TRUE;
      }
    }
#endif

    if (!spr_mask)
    {
      if (colour_type == PNG_COLOR_TYPE_GRAY)
        png_set_gray_to_rgb(png_ptr);

#ifndef NDEBUG
      fprintf(stderr, "png: expanding transparency to full alpha\n");
#endif
      png_set_tRNS_to_alpha(png_ptr);
      has_alpha = TRUE;
      sprite_bpp = 32;
    }
  }

#if 0
  {
    //png_color_16 my_background;
    png_color_16p image_background;

    if (png_get_bKGD(png_ptr, info_ptr, &image_background))
    {
      png_set_background(png_ptr, image_background,
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);

      /* this will drop the alpha data so will need filler */

      png_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
      has_alpha = FALSE;
    }
    /*else
        png_set_background(png_ptr, &my_background,
          PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);*/
  }
#else
  {
    png_color_16p bg;
    os_colour     colour = os_COLOUR_TRANSPARENT;

    if (png_get_bKGD(png_ptr, info_ptr, &bg))
    {
      /* the png_color values depends on the bit_depth of the image. */

      switch (bit_depth)
      {
      case 8:
        /* these & 0xffs seem to be necessary. not sure why. */

        colour = ((bg->red   & 0xff) << 8 ) |
                 ((bg->green & 0xff) << 16) |
                 ((bg->blue  & 0xff) << 24);
        break;

      case 16:
        colour = ((bg->red   & 0xff00)     ) |
                 ((bg->green & 0xff00) << 8) |
                 ((bg->blue  & 0xff00) << 16);
        break;
      }
    }

    image->background_colour = colour;
  }
#endif

  {
    png_uint_32 res_x, res_y;
    int         unit_type;

    source_xdpi = source_ydpi = 0;

    if (png_get_pHYs(png_ptr, info_ptr, &res_x, &res_y, &unit_type))
    {
      if (unit_type == PNG_RESOLUTION_METER)
      {
        /* convert dots/metre to dpi */
        source_xdpi = (int)(res_x * .0254 + .50);
        source_ydpi = (int)(res_y * .0254 + .50);
      }
    }
  }

  if (sprite_bpp <= 8)
  {
    /* 1bpp up to 8bpp */
#ifndef NDEBUG
    fprintf(stderr, "1bpp up to 8bpp\n");
#endif
    sprite_palbytes = 1 << (sprite_bpp + 3);
  }
  else
  {
    /* 32bpp (there is no 16-bit format) */
#ifndef NDEBUG
    fprintf(stderr, "32bpp\n");
#endif
    sprite_palbytes = 0;
  }

  mode = sprite_mode(1, 1, (int) floorlog2(sprite_bpp));

  sprite_rowbytes = (int) (((width * sprite_bpp + 31) & ~31) >> 3);
  sprite_imgbytes = 16 + 44 + sprite_palbytes + sprite_rowbytes * (int) height;

  if (spr_mask)
  {
    int sprite_rowmaskbytes;

    if (((osspriteop_mode_word) mode & osspriteop_TYPE) == osspriteop_TYPE_OLD) /* old format */
      sprite_rowmaskbytes = sprite_rowbytes;
    else /* new format */
      // will this case get used?
      sprite_rowmaskbytes = (int) (((width * 1 + 31) & ~31) >> 3);

    sprite_imgbytes += sprite_rowmaskbytes * (int) height;
  }

  if (flex_alloc((flex_ptr) &area, sprite_imgbytes) == 0)
    goto NoMem;

  area->size = sprite_imgbytes;
  area->first = 16;
  osspriteop_clear_sprites(osspriteop_USER_AREA, area);

#ifndef NDEBUG
  fprintf(stderr, "creating sprite; %d\n", (int) mode);
#endif
  osspriteop_create_sprite(osspriteop_NAME, area, "png", 0 /* unpaletted */, (int) width, (int) height, mode);

  header = sprite_select(area, 0);

  if (sprite_bpp <= 8)
  {
    unsigned int *sprite_palette;

    osspriteop_create_true_palette(osspriteop_PTR, area, (osspriteop_id) header);
    sprite_palette = (unsigned int *) (header + 1);

#ifndef NDEBUG
    fprintf(stderr, "adding palette\n");
#endif

    /* generate a grey palette in all cases so that even if we receive a
     * smaller than expected palette, it's still all filled out */
    make_grey_palette((int) floorlog2(sprite_bpp), sprite_palette);

    if (png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette) != 0)
    {
      /* move the palette entries across */

      for (i = num_palette - 1; i >= 0 ; i--)
        sprite_palette[i * 2 + 0] =
        sprite_palette[i * 2 + 1] = (palette[i].blue << 24) | (palette[i].green << 16) | (palette[i].red << 8) | 16;
    }
  }

  sprite_ptr = sprite_data(header);

  /* load the image */

  if (flex_alloc((flex_ptr) &row_pointers, (int) (sizeof(png_bytep) * height)) == 0)
    goto NoMem;

  /* we've just claimed memory, so the heap may have moved */
  header = sprite_select(area, 0);
  sprite_ptr = (byte *) (header + 1) + sprite_palbytes;

  /* set up the row pointers */
  for (h = 0; h < height; h++)
     row_pointers[h] = (png_bytep) (sprite_ptr + sprite_rowbytes * h);
  png_read_image(png_ptr, row_pointers);

  flex_free((flex_ptr) &row_pointers);

  if (spr_mask)
  {
    osspriteop_create_mask(osspriteop_PTR, area, (osspriteop_id) header);

    sprite_mask_pixel(area, header, trans[0]);
  }

  png_read_end(png_ptr, info_ptr);

  png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);

  fclose(fp);

  image->flags = 0;

  switch (colour_type)
  {
    case PNG_COLOR_TYPE_GRAY:
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      /* monochrome implied by absence of image_FLAG_COLOUR */
      break;
    case PNG_COLOR_TYPE_PALETTE:
    case PNG_COLOR_TYPE_RGB:
    case PNG_COLOR_TYPE_RGB_ALPHA:
      image->flags = image_FLAG_COLOUR;
      break;
  }

  if (((unsigned int) mode >> osspriteop_TYPE_SHIFT) == osspriteop_TYPE32BPP)
    image->flags |= image_FLAG_CAN_HIST;

  image->flags            |= image_FLAG_CAN_ROT;

  if (spr_mask)
    image->flags          |= image_FLAG_HAS_MASK;

  if (has_alpha)
    image->flags          |= image_FLAG_HAS_ALPHA;

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &area);


  image->display.dims.bm.width  = (int) width;
  image->display.dims.bm.height = (int) height;
  read_mode_vars(mode, &image->display.dims.bm.xeig,
                       &image->display.dims.bm.yeig,
                       &log2bpp);
  image->display.dims.bm.xdpi   = os_INCH >> image->display.dims.bm.xeig;
  image->display.dims.bm.ydpi   = os_INCH >> image->display.dims.bm.yeig;
  image->display.dims.bm.bpp    = 1 << log2bpp;
  image->display.file_size      = sprite_imgbytes;
  image->display.file_type      = osfile_TYPE_SPRITE;

  image->source.dims            = image->display.dims; /* copy */
  image->source.dims.bm.xdpi    = source_xdpi;
  image->source.dims.bm.ydpi    = source_ydpi;
  image->source.dims.bm.bpp     = source_bpp;
  image->source.load            = source_load;
  image->source.exec            = source_exec;
  image->source.file_size       = source_file_size;
  image->source.file_type       = png_FILE_TYPE;

  image->scale.min = 1;
  image->scale.max = 32767;

  image->details.sprite.mode      = mode;


  hourglass_off();

  return FALSE; /* success */


 CleanUp:
  if (png_ptr)
    png_destroy_read_struct(&png_ptr, (png_infopp) NULL, (png_infopp) NULL);

  if (fp)
    fclose(fp);

  hourglass_off();

  return TRUE; /* failure */


 NoMem:

  oserror__report(0, "error.no.mem");

  goto CleanUp;
}

void png_export_methods(image_choices *choices, image *image)
{
  static const image_methods methods =
  {
    png_load,
    bitmap_save,
    bitmap_unload,
    bitmap_histogram,
    bitmap_rotate,
    NULL
  };

  NOT_USED(choices);

  image->methods = methods;
}
