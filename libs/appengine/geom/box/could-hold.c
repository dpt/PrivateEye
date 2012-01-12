
#include "oslib/os.h"

#include "appengine/types.h"

#include "appengine/geom/box.h"

int box_could_hold(const os_box *b, int w, int h)
{
  return (b->x1 - b->x0) >= w &&
         (b->y1 - b->y0) >= h;
}
