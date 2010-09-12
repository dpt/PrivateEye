/* $Id: set-submenu-extent.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_set_submenu_extent(wimp_w w, unsigned int flags, os_box *ubox)
{
  wimp_window_info info;
  os_box           box;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  box.x0 = (flags & 1) ? ubox->x0 : info.extent.x0;
  box.y0 = (flags & 2) ? ubox->y0 : info.extent.y0;
  box.x1 = (flags & 4) ? ubox->x1 : info.extent.x1;
  box.y1 = (flags & 8) ? ubox->y1 : info.extent.y1;
  wimp_set_extent(w, &box);

  /* When the window is later opened by the Wimp as a submenu it won't be
   * expanded to the right size, so we now open up the window behind the
   * desktop's back window so we can apply the right size. */

  if ((info.flags & wimp_WINDOW_OPEN) == 0)
  {
    wimp_open open;

    open.w       = w;
    open.visible = box;
    open.xscroll = 0;
    open.yscroll = 0;
    open.next    = wimp_HIDDEN; /* open behind the back window */
    wimp_open_window(&open);
  }
}
