
#include "oslib/os.h"

#include "appengine/geom/box.h"

/* increases the size of box "box" by "change" */

void box_grow(os_box *box, int change)
{
  box->x0 -= change;
  box->y0 -= change;
  box->x1 += change;
  box->y1 += change;
}
