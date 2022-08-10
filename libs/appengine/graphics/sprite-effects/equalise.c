/* --------------------------------------------------------------------------
 *    Name: equalise.c
 * Purpose: Histogram equalisation
 * ----------------------------------------------------------------------- */

#include <string.h>

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"
#include "appengine/vdu/sprite.h"

#include "appengine/graphics/sprite-effects.h"

result_t effects_equalise_apply(osspriteop_area   *area,
                                osspriteop_header *src,
                                osspriteop_header *dst)
{
  result_t          err;
  sprite_histograms hists;
  int               i;
  sprite_luts       luts;

  err = sprite_get_histograms(area, src, &hists);
  if (err)
    return err;

  for (i = 0; i < 3; i++)
  {
    int t;
    int j;
    int scale;

    /* make cumulative */
    t = 0;
    for (j = 0; j < 256; j++)
      hists.h[i + 1].v[j] = (t += hists.h[i + 1].v[j]);

    /* t = no. of pixels (weighted) */

    /* scale map */

    scale = (255 << 16) / t;

    for (j = 0; j < 256; j++)
      luts.l[i].v[j] = (hists.h[i + 1].v[j] * scale) >> 16;
  }

  err = sprite_remap(area, src, dst, &luts);
  if (err)
    return err;

  return result_OK;
}
