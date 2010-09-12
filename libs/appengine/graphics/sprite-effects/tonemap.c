/* --------------------------------------------------------------------------
 *    Name: tonemap.c
 * Purpose: Tone map effect
 * Version: $Id: tonemap.c,v 1.3 2009-05-21 22:27:20 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "appengine/base/utils.h"

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

#include "appengine/graphics/sprite-effects.h"

// FIXME: use version in sprite if/when it can cope with 4 tables
static void tonemap_888(osspriteop_header         *src,
                        osspriteop_header         *dst,
                        int                        width,
                        int                        height,
                        const os_correction_table *red,
                        const os_correction_table *green,
                        const os_correction_table *blue,
                        const os_correction_table *alpha)
{
  int           xy;
  unsigned int *sp;
  unsigned int *dp;

  sp = sprite_data(src);
  dp = sprite_data(dst);

  for (xy = width * height; xy--; )
  {
    unsigned int spx;

    /* 0xAABBGGRR */

    spx = *sp++;

    *dp++ = (  red->gamma[(spx >> 0)  & 0xff] << 0 ) |
            (green->gamma[(spx >> 8)  & 0xff] << 8 ) |
            ( blue->gamma[(spx >> 16) & 0xff] << 16) |
            (alpha->gamma[(spx >> 24) & 0xff] << 24);
  }
}

error effects_tonemap_apply(osspriteop_area   *area,
                            osspriteop_header *src,
                            osspriteop_header *dst,
                            tonemap           *map)
{
  const os_correction_table *red, *green, *blue, *alpha;
  int                        width, height;
  osspriteop_mode_word       mode;
  int                        log2bpp;

  tonemap_get_corrections(map, &red, &green, &blue, &alpha);

  sprite_info(area, src, &width, &height, NULL, (os_mode *) &mode, &log2bpp);

  switch (log2bpp)
  {
  case 5:
    tonemap_888(src, dst, width, height, red, green, blue, alpha);
    break;

  default:
    return error_SPRITEFX_UNSUPP_EFFECT;
  }

  return error_OK;
}
