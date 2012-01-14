
#include "oslib/os.h"

#include "appengine/geom/box.h"

/* return true if box "a" overlaps box "b" */

int box_intersects(const os_box *a, const os_box *b)
{
  return a->x0 < b->x1 && a->x1 > b->x0 &&
         a->y0 < b->y1 && a->y1 > b->y0;
}
