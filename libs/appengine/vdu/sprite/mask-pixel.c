/* --------------------------------------------------------------------------
 *    Name: mask-pixel.c
 * Purpose: Adds a mask covering the specified pixel value
 * Version: $Id: mask-pixel.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

typedef void (mktabfn)(unsigned char lut[256], int value);

// mask bits are clear if masked, set if unmasked
#if 0
static void mask_32bpp_new(osspriteop_header *header,
                           unsigned int       value)
{
  unsigned int *sp;
  unsigned int *mp;
  int           xy;
  int           i = 0;

  sp = sprite_data(header);
  mp = sprite_mask_data(header);

//  mask_rowbytes = (int) (((width * 1 + 31) & ~31) >> 3);

  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      unsigned int p;
      unsigned int m;

      m = 0xffffffff;

      /* 32 pixels = 1 mask word */

      if (*sp++ == value)
        m &= ~(1 << i);

      if (++i == 32)
      {
        *mp++ = m;
        i = 0;
      }
    }

    *mp++ = m; // what should unused mask values be set to?
    i = 0;
  }
}
#endif

static void mktab_1bpp_old(unsigned char lut[256], int value)
{
  int i;

  for (i = 0; i < 256; i++)
    lut[i] = (value == 0) ? i : ~i;
}

static void mktab_2bpp_old(unsigned char lut[256], int value)
{
  int i;

  for (i = 0; i < 256; i++)
  {
    int m = 0xff;

    if (((i >> 0) & 3) == value) m &= ~(3 << 0);
    if (((i >> 2) & 3) == value) m &= ~(3 << 2);
    if (((i >> 4) & 3) == value) m &= ~(3 << 4);
    if (((i >> 6) & 3) == value) m &= ~(3 << 6);

    lut[i] = m;
  }
}

static void mktab_4bpp_old(unsigned char lut[256], int value)
{
  int i;

  for (i = 0; i < 256; i++)
  {
    int m = 0xff;

    if (((i >> 0) & 0xf) == value) m &= ~(0xf << 0);
    if (((i >> 4) & 0xf) == value) m &= ~(0xf << 4);

    lut[i] = m;
  }
}

static void mktab_8bpp_old(unsigned char lut[256], int value)
{
  memset(lut, 0xff, 256); /* set all opaque */
  lut[value] = 0x00; /* except for one transparent */
}

void sprite_mask_pixel(osspriteop_area   *area,
                       osspriteop_header *header,
                       unsigned int       value)
{
  static const mktabfn *makers[] =
  {
    mktab_1bpp_old,
    mktab_2bpp_old,
    mktab_4bpp_old,
    mktab_8bpp_old,
    NULL,
    NULL,
  };

  osspriteop_mode_word mode;

  NOT_USED(area);

  mode = (osspriteop_mode_word) header->mode;

  if ((mode & osspriteop_TYPE) == osspriteop_TYPE_OLD)
  {
    int            log2bpp;
    const mktabfn *mktab;
    unsigned char  lut[256];
    unsigned int  *sp;
    unsigned int  *mp;
    int            xy;

    os_read_mode_variable((os_mode) mode, os_MODEVAR_LOG2_BPP, &log2bpp);

    mktab = makers[log2bpp];
    if (mktab == NULL)
    {
      assert("sprite_mask_pixel doesn't support wacky sprites." == NULL);
      return;
    }

    mktab(lut, value);

    sp = sprite_data(header);
    mp = sprite_mask_data(header);

    for (xy = (header->width + 1) * (header->height + 1); xy != 0; xy--)
    {
      unsigned int p;
      unsigned int m;

      p = *sp++;

      /* look up one byte at a time: 4 steps irrespective of bpp */

      m  = lut[(p >>  0) & 0xff] << 0;
      m |= lut[(p >>  8) & 0xff] << 8;
      m |= lut[(p >> 16) & 0xff] << 16;
      m |= lut[(p >> 24) & 0xff] << 24;

      *mp++ = m;
    }
  }
  else
  {
    assert("sprite_mask_pixel doesn't support new style sprites." == NULL);
  }
}
