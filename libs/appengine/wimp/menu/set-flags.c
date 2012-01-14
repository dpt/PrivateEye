
#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_set_menu_flags(wimp_menu      *menu,
                         int             entry,
                         wimp_menu_flags eor_bits,
                         wimp_menu_flags clear_bits)
{
  menu->entries[entry].menu_flags =
    (menu->entries[entry].menu_flags & ~clear_bits) ^ eor_bits;
}
