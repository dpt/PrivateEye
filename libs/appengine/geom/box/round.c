/* $Id: round.c,v 1.1 2009-05-18 22:25:31 dpt Exp $ */

#include "oslib/os.h"

#include "appengine/geom/box.h"

void box__round(os_box *a, int amount)
{
  // not fully tested
  a->x0 = ((a->x0 - (amount - 1)) / amount) * amount;
  a->y0 = ((a->y0 - (amount - 1)) / amount) * amount;
  a->x1 = ((a->x1 + (amount - 1)) / amount) * amount;
  a->y1 = ((a->y1 + (amount - 1)) / amount) * amount;
}
