/* $Id: open-as-menu-here.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_open_as_menu_here(wimp_w w, int x, int y)
{
  wimp_create_menu((wimp_menu *) w, x, y);
}
