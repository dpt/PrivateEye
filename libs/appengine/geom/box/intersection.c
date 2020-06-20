
#include "oslib/os.h"

#include "appengine/types.h"

#include "appengine/geom/box.h"

/* calculate box "c" as the intersection of boxes "a" and "b" */
/* returns true if result is invalid */

int box_intersection(const os_box *a, const os_box *b, os_box *c)
{
  c->x0 = MAX(a->x0, b->x0);
  c->y0 = MAX(a->y0, b->y0);
  c->x1 = MIN(a->x1, b->x1);
  c->y1 = MIN(a->y1, b->y1);

  return box_is_empty(c);
}
