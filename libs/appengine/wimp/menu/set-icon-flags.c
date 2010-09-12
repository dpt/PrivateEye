/* $Id: set-icon-flags.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_set_icon_flags(wimp_menu       *menu,
                         int              entry,
                         wimp_icon_flags  eor_bits,
                         wimp_icon_flags  clear_bits)
{
  menu->entries[entry].icon_flags = (menu->entries[entry].icon_flags & ~clear_bits) ^ eor_bits;
}
