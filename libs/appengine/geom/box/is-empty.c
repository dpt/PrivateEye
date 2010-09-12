/* $Id: is-empty.c,v 1.2 2010-01-13 18:13:07 dpt Exp $ */

#include "oslib/os.h"

#include "appengine/geom/box.h"

int box__is_empty(const os_box *a)
{
  return (a->x0 >= a->x1) || (a->y0 >= a->y1);
}
