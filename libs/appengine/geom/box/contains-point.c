/* $Id: contains-point.c,v 1.1 2009-05-18 22:25:31 dpt Exp $ */

#include "oslib/os.h"

#include "appengine/geom/box.h"

/* return true if box "box" is contains the point "x","y" */

int box__contains_point(const os_box *box, int x, int y)
{
  return x >= box->x0 && x <= box->x1 && y >= box->y0 && y <= box->y1;
}
