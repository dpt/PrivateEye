/* --------------------------------------------------------------------------
 *    Name: vector.c
 * Purpose: Generic vector viewer methods
 * Version: $Id: drawable-vector.c,v 1.1 2009-04-28 23:32:24 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/draw.h"
#include "oslib/os.h"

#include "appengine/graphics/drawable.h"

#include "drawable-vector.h"

void vector_scaling(drawable *drawable, const os_factors *factors)
{
  os_trfm *trfm;

  trfm = &drawable->details.generic.trfm;

  trfm->entries[0][0] = (factors->xmul << 16) / factors->xdiv;
  trfm->entries[0][1] = 0;
  trfm->entries[1][0] = 0;
  trfm->entries[1][1] = (factors->ymul << 16) / factors->ydiv;
}

void vector_get_dimensions(drawable *drawable, const os_factors *factors, os_box *box)
{
  os_factors f;
  int        xdiv, ydiv;
  os_box    *bbox;

  f = *factors; /* local copy to avoid aliasing */

  xdiv = draw_OS_UNIT * f.xdiv;
  ydiv = draw_OS_UNIT * f.ydiv;

  bbox = &drawable->image->display.dims.vc.box;

  box->x0 = (bbox->x0 * f.xmul) / xdiv;
  box->y0 = (bbox->y0 * f.ymul) / ydiv;
  box->x1 = (bbox->x1 * f.xmul) / xdiv;
  box->y1 = (bbox->y1 * f.ymul) / ydiv;
}
