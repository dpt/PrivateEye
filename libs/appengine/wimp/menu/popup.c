/* $Id: popup.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_popup(wimp_w w, wimp_i i, wimp_menu *menu)
{
  wimp_window_state window;
  wimp_icon_state   icon;
  int               x,y;

  window.w = w;
  wimp_get_window_state(&window);

  icon.w = w;
  icon.i = i;
  wimp_get_icon_state(&icon);

  x = window.visible.x0 - window.xscroll + icon.icon.extent.x1 + 2;
  y = window.visible.y1 - window.yscroll + icon.icon.extent.y1;

  menu_open(menu, x, y);
}
