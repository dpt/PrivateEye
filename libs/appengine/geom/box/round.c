
#include "oslib/os.h"

#include "appengine/geom/box.h"

void box_round(os_box *a, int log2x, int log2y)
{
  const int x = (1 << log2x) - 1;
  const int y = (1 << log2y) - 1;
  os_box    b = *a;

  b.x0 = (b.x0    ) & ~x;
  b.y0 = (b.y0    ) & ~y;
  b.x1 = (b.x1 + x) & ~x;
  b.y1 = (b.y1 + y) & ~y;

  *a = b;
}
