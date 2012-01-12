
#include "oslib/os.h"

#include "appengine/geom/box.h"

void box_round4(os_box *a)
{
  os_box b = *a;

  b.x0 = (b.x0    ) & ~3;
  b.y0 = (b.y0    ) & ~3;
  b.x1 = (b.x1 + 3) & ~3;
  b.y1 = (b.y1 + 3) & ~3;

  *a = b;
}
