/* $Id: set-submenu.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_set_submenu(wimp_menu *menu, int entry, void *sub_menu)
{
  menu->entries[entry].sub_menu = sub_menu;
}
