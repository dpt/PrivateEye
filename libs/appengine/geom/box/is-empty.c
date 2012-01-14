
#include "oslib/os.h"

#include "appengine/geom/box.h"

int box_is_empty(const os_box *a)
{
  return (a->x0 >= a->x1) ||
         (a->y0 >= a->y1);
}
