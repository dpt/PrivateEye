
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_set_submenu_extent(wimp_w w, unsigned int flags, os_box *ubox)
{
  wimp_window_info info;
  os_box           newextent;
  wimp_open        open;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  newextent.x0 = (flags & 1) ? ubox->x0 : info.extent.x0;
  newextent.y0 = (flags & 2) ? ubox->y0 : info.extent.y0;
  newextent.x1 = (flags & 4) ? ubox->x1 : info.extent.x1;
  newextent.y1 = (flags & 8) ? ubox->y1 : info.extent.y1;
  wimp_set_extent(w, &newextent);

  /* When the window is later opened by the Wimp as a submenu it won't be
   * expanded to the right size, so if it's not already open we now open up
   * the window behind the desktop's back window so we can apply the right
   * size. */
  open.w = w;
  open.xscroll = 0;
  open.yscroll = 0;
  if ((info.flags & wimp_WINDOW_OPEN) == 0)
  {
    open.visible = newextent;
    open.next = wimp_HIDDEN; /* open behind the back window */
    wimp_open_window(&open);
  }
  else
  {
    open.visible.x0 = info.visible.x0;
    open.visible.y0 = info.visible.y1 - (info.extent.y1 - info.extent.y0);
    open.visible.x1 = info.visible.x0 + (info.extent.x1 - info.extent.x0);
    open.visible.y1 = info.visible.y1;
    open.next = info.next;
    wimp_open_window(&open);
    wimp_force_redraw(w, newextent.x0, newextent.y0, newextent.x1, newextent.y1);
  }
}
