
#include "oslib/os.h"

#include "appengine/geom/box.h"

/* return true if box "box" is contains the point "x","y" */

int box__contains_point(const os_box *box, int x, int y)
{
  return x >= box->x0 && x <= box->x1 && y >= box->y0 && y <= box->y1;
}
