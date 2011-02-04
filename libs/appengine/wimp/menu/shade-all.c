
#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_shade_all(wimp_menu *menu)
{
  wimp_menu_entry *item;

  item = menu->entries;

  do
    item->icon_flags |= wimp_ICON_SHADED;
  while (((item++)->menu_flags & wimp_MENU_LAST) == 0);
}
