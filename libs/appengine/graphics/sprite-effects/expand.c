/* --------------------------------------------------------------------------
 *    Name: expand.c
 * Purpose: Expand dynamic range
 * Version: $Id: expand.c,v 1.3 2009-05-21 22:27:20 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"
#include "appengine/vdu/sprite.h"

#include "appengine/graphics/sprite-effects.h"

/* FIXME: Threshold should use a percentage rather than literal count. */

error effects_expand_apply(osspriteop_area   *area,
                           osspriteop_header *src,
                           osspriteop_header *dst,
                           int                threshold)
{
  error             err;
  sprite_histograms hists;
  int               min;
  int               max;
  int               i;
  int               j;
  sprite_luts       luts;
  int               nbins;
  double            v;
  unsigned char    *l;

  err = sprite_get_histograms(area, src, &hists);
  if (err)
    return err;

  /* find the minima and maxima for all of the R,G,B histograms */

  min = 255;
  max = 0;

  for (i = 0; i < 3; i++)
  {
    unsigned int *h;

    h = &hists.h[i + 1].v[0]; /* i+1 to skip initial luma hist */

    for (j = 0; j < 256; j++)
      if (h[j] > threshold)
        break;

    if (j < min)
      min = j;

    for (j = 255; j >= 0; j--)
      if (h[j] > threshold)
        break;

    if (j > max)
      max = j;
  }

  /* I'd like at this point to say:
   *
   *   if (min == 0 && max == 255)
   *     return error_OK; // nothing to expand
   *
   * but reckon that'll stop sprite_remap degenerating into a straight copy
   * in that case, and so might screw up the expected result.
   */

  memset(&luts.l[0], 0, sizeof(luts.l[0]));

  nbins = max + 1 - min;
  v = 255.0 / (nbins - 1.0);

  /* min,max are indices of the first,last non-zero buckets */

  l = &luts.l[0].v[min];
  for (j = 0; j < nbins; j++)
    l[j] = (int) (v * j);

  memcpy(&luts.l[1], &luts.l[0], sizeof(luts.l[0]));
  memcpy(&luts.l[2], &luts.l[0], sizeof(luts.l[0]));

  if (0)
  {
    for (j = 0; j < 256; j++)
      fprintf(stderr, "%d:%d\n", j, luts.l[0].v[j]);
  }

  err = sprite_remap(area, src, dst, &luts);
  if (err)
    return err;

  return error_OK;
}
