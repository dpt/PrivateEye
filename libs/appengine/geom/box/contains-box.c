/* $Id: contains-box.c,v 1.1 2009-05-18 22:25:31 dpt Exp $ */

#include "oslib/os.h"

#include "appengine/geom/box.h"

/* return true if box "inside" is contained by box "outside" */

int box__contains_box(const os_box *inside, const os_box *outside)
{
  return inside->x0 >= outside->x0 &&
         inside->y0 >= outside->y0 &&
         inside->x1 <= outside->x1 &&
         inside->y1 <= outside->y1;
}
