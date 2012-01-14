/* --------------------------------------------------------------------------
 *    Name: remap-luma.c
 * Purpose: Remap Luma sprite pixels
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/types.h"
#include "appengine/vdu/sprite.h"

error sprite_remap_luma(osspriteop_area   *area,
                        osspriteop_header *src,
                        osspriteop_header *dst,
                        sprite_lut        *lut)
{
  int                  width, height;
  osspriteop_mode_word mode;
  int                  log2bpp;
  unsigned int        *sp;
  unsigned int        *dp;
  int                  xy;

  sprite_info(area,
              src,
             &width, &height,
              NULL,
 (os_mode *) &mode,
             &log2bpp);

  switch (log2bpp)
  {
  case 5:
    break;

  default:
    return error_SPRITEFX_UNSUPP_FUNC;
  }

  sp = sprite_data(src);
  dp = sprite_data(dst);

  for (xy = width * height; xy--; )
  {
    unsigned int spx;
    int          r, g, b;
    int          y, cb, cr;

    /* 0xAABBGGRR */

    spx = *sp++;

    r = (spx >> 0)  & 0xff;
    g = (spx >> 8)  & 0xff;
    b = (spx >> 16) & 0xff;

    /* using JPEG weights (see libjpeg's jccolor.c and jdcolor.c) */
    /* obviously this all needs speeding up with precalculated tables. */

    y  = ((  r * 19595 + g * 38470 + b *  7471) >> 16);
    cb = ((- r * 11058 - g * 21710 + b * 32768) >> 16) + 128;
    cr = ((  r * 32768 - g * 27439 - b *  5329) >> 16) + 128;

    y = lut->v[y];

    cb -= 128;
    cr -= 128;
    r = (y * 65536               + cr * 91881) >> 16;
    g = (y * 65536 - cb *  22554 - cr * 46802) >> 16;
    b = (y * 65536 + cb * 116130             ) >> 16;

    // may need to clamp
    r = CLAMP(r, 0, 255);
    g = CLAMP(g, 0, 255);
    b = CLAMP(b, 0, 255);

    // problem: this routine appears to generate a green shift when no change
    //          is applied.

    *dp++ = (r << 0) | (g << 8) | (b << 16) | (spx & 0xff00000);
  }

  return error_OK;
}
