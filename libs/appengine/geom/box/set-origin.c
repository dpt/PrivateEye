
#include "oslib/os.h"

#include "appengine/types.h"

#include "appengine/geom/box.h"

/* move box b to new origin (x0,y0) */

void box_set_origin(os_box *b, int x0, int y0)
{
  int dx = x0 - b->x0;
  int dy = y0 - b->y0;

  b->x0 += dx;
  b->y0 += dy;
  b->x1 += dx;
  b->y1 += dy;
}
