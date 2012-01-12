
#include "oslib/os.h"

#include "appengine/types.h"

#include "appengine/geom/box.h"

/* return a box "c" that contains both boxes "a" and "b" */

void box_union(const os_box *a, const os_box *b, os_box *c)
{
  c->x0 = MIN(a->x0, b->x0);
  c->y0 = MIN(a->y0, b->y0);
  c->x1 = MAX(a->x1, b->x1);
  c->y1 = MAX(a->y1, b->y1);
}
