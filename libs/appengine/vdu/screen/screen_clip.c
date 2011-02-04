
#include "oslib/os.h"

#include "appengine/vdu/screen.h"

/* coords are OS units, x1 and y1 are exclusive */

os_error *screen_clip(const os_box *b)
{
  int c;
  char stream[9];

  stream[0] = 24;

  c = b->x0;

  stream[1] = c;
  stream[2] = c >> 8;

  c = b->y0;

  stream[3] = c;
  stream[4] = c >> 8;

  c = b->x1 - 1; /* make inclusive */

  stream[5] = c;
  stream[6] = c >> 8;

  c = b->y1 - 1; /* make inclusive */

  stream[7] = c;
  stream[8] = c >> 8;

  return xos_writen(stream, 9);
}
