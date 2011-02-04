/* --------------------------------------------------------------------------
 *    Name: grey.c
 * Purpose: Greyscale effect
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

#include "appengine/graphics/sprite-effects.h"

static void grey_888(osspriteop_header         *src,
                     osspriteop_header         *dst,
                     int                        width,
                     int                        height)
{
#if 1
  /* libjpeg's weights */
  const int red_weight   = 19595; /* 0.29900 * 65536 (rounded down) */
  const int green_weight = 38470; /* 0.58700 * 65536 (rounded up)   */
  const int blue_weight  =  7471; /* 0.11400 * 65536 (rounded down) */
#else
  /* libpng's weights */
  const int red_weight   = 13938; /* 0.212671 * 65536 (rounded up)   */
  const int green_weight = 46869; /* 0.715160 * 65536 (rounded up)   */
  const int blue_weight  =  4729; /* 0.072169 * 65536 (rounded down) */
#endif

  int           xy;
  unsigned int *sp;
  unsigned int *dp;

  sp = sprite_data(src);
  dp = sprite_data(dst);

  for (xy = width * height; xy--; )
  {
    unsigned int spx;
    int          r, g, b;
    int          gr;

    /* 0xAABBGGRR */

    spx = *sp++;

    r = (spx >> 0)  & 0xff;
    g = (spx >> 8)  & 0xff;
    b = (spx >> 16) & 0xff;

    if (r == g && g == b)
      gr = r;
    else
      gr = (r * red_weight + g * green_weight + b * blue_weight) >> 16;

    *dp++ = (gr << 0 ) | (gr << 8 ) | (gr << 16) | (spx & 0xff000000);
  }
}

error effects_grey_apply(osspriteop_area   *area,
                         osspriteop_header *src,
                         osspriteop_header *dst)
{
  int                        width, height;
  osspriteop_mode_word       mode;
  int                        log2bpp;

  sprite_info(area, src, &width, &height, NULL, (os_mode *) &mode, &log2bpp);

  switch (log2bpp)
  {
  case 5:
    grey_888(src, dst, width, height);
    break;

  default:
    return error_SPRITEFX_UNSUPP_EFFECT;
  }

  return error_OK;
}
