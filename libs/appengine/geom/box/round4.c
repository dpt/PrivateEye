/* $Id: round4.c,v 1.1 2009-05-18 22:25:31 dpt Exp $ */

#include "oslib/os.h"

#include "appengine/geom/box.h"

void box__round4(os_box *a)
{
  os_box b = *a;

  b.x0 = (b.x0    ) & ~3;
  b.y0 = (b.y0    ) & ~3;
  b.x1 = (b.x1 + 3) & ~3;
  b.y1 = (b.y1 + 3) & ~3;

  *a = b;
}
