
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

int window_set_extent2(wimp_w w, int xmin, int ymin, int xmax, int ymax)
{
  os_box             box;
  union
  {
    wimp_window_info info;
    wimp_open        open;
  }
  block;
  int                larger_than_requested;

  box.x0 = xmin;
  box.y0 = ymin;
  box.x1 = xmax;
  box.y1 = ymax;

  wimp_set_extent(w, &box);

  /* FIXME: If this executes when the function is being called from a
   *        mode change handler, then the window gets fixed in position
   *        off-screen.
   */

  block.info.w = w;
  wimp_get_window_info_header_only(&block.info);

  if ((block.info.extent.x1 - block.info.extent.x0 > xmax - xmin) ||
      (block.info.extent.y1 - block.info.extent.y0 > ymax - ymin))
  {
    /* If the window is larger than we want (i.e. it's reached its smallest
     * size) then set the 'clear background' flag and resize the window to
     * the size it wants to be, so we don't get any redraw glitches. */

    wimp_set_extent(w, &block.info.extent);

    larger_than_requested = 1;
  }
  else
  {
    larger_than_requested = 0;
  }

  if (block.info.flags & wimp_WINDOW_OPEN)
    wimp_open_window(&block.open);

  return larger_than_requested;
}
