/* --------------------------------------------------------------------------
 *    Name: remap.c
 * Purpose: Sprite remapping
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"

#include "appengine/vdu/sprite.h"

result_t sprite_remap(osspriteop_area   *area,
                   osspriteop_header *src,
                   osspriteop_header *dst,
                   sprite_luts       *luts)
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
    return result_SPRITEFX_UNSUPP_FUNC;
  }

  sp = sprite_data(src);
  dp = sprite_data(dst);

  for (xy = width * height; xy--; )
  {
    unsigned int p;

    /* 0xXXBBGGRR */

    p = *sp++;

    *dp++ = (luts->l[0].v[((p >> 0)  & 0xff)] << 0 ) |
            (luts->l[1].v[((p >> 8)  & 0xff)] << 8 ) |
            (luts->l[2].v[((p >> 16) & 0xff)] << 16) |
            (p & 0xff000000);
  }

  return result_OK;
}
