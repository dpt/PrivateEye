
#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

int menu_count(const wimp_menu *menu)
{
  const wimp_menu_entry *item;

  item = menu->entries;
  while (((item++)->menu_flags & wimp_MENU_LAST) == 0)
    ;

  return item - menu->entries;
}
