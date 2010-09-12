/* $Id: could-hold.c,v 1.1 2009-05-18 22:25:31 dpt Exp $ */

#include "oslib/os.h"

#include "appengine/types.h"

#include "appengine/geom/box.h"

int box__could_hold(const os_box *b, int w, int h)
{
  return (b->x1 - b->x0) >= w &&
         (b->y1 - b->y0) >= h;
}
