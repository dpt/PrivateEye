
#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_set_icon_flags(wimp_menu       *menu,
                         int              entry,
                         wimp_icon_flags  eor_bits,
                         wimp_icon_flags  clear_bits)
{
  menu->entries[entry].icon_flags = (menu->entries[entry].icon_flags & ~clear_bits) ^ eor_bits;
}
