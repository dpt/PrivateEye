/* $Id: open.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

/* Statics for menu_(re)open */
static wimp_menu *last_menu = NULL;

void menu_open(wimp_menu *menu, int x, int y)
{
  last_menu = menu;
  wimp_create_menu(menu, x, y);
}

void menu_reopen(void)
{
  if (last_menu)
    wimp_create_menu(last_menu, 0, 0);
}

wimp_menu *menu_last(void)
{
  return last_menu;
}

