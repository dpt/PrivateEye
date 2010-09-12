/* $Id: get-screen-bbox.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_get_screen_bbox(wimp_w w, wimp_i i, os_box *bbox)
{
  wimp_window_state wblock;
  wimp_icon_state   iblock;

  wblock.w = w;
  wimp_get_window_state(&wblock);

  iblock.w = w;
  iblock.i = i;
  wimp_get_icon_state(&iblock);

  bbox->x0 = wblock.visible.x0 - wblock.xscroll + iblock.icon.extent.x0;
  bbox->y0 = wblock.visible.y1 - wblock.yscroll + iblock.icon.extent.y0;
  bbox->x1 = bbox->x0 + iblock.icon.extent.x1 - iblock.icon.extent.x0;
  bbox->y1 = bbox->y0 + iblock.icon.extent.y1 - iblock.icon.extent.y0;
}
