/* $Id: tick-exclusive.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_tick_exclusive(wimp_menu *menu, int entry_to_tick)
{
  wimp_menu_entry *item;
  wimp_menu_entry *tick;

  item = menu->entries;
  tick = menu->entries + entry_to_tick;

  do
  {
    if (item == tick)
      item->menu_flags |= wimp_MENU_TICKED;
    else
      item->menu_flags &= ~wimp_MENU_TICKED;
  }
  while (((item++)->menu_flags & wimp_MENU_LAST) == 0);
}

void menu_range_tick_exclusive(wimp_menu *menu,
                               int        entry_to_tick,
                               int        low,
                               int        high)
{
  wimp_menu_entry *end;
  wimp_menu_entry *tick;
  wimp_menu_entry *item;

  end  = menu->entries + high;
  tick = menu->entries + entry_to_tick;

  for (item = menu->entries + low; item <= end; item++)
  {
    if (item == tick)
      item->menu_flags |= wimp_MENU_TICKED;
    else
      item->menu_flags &= ~wimp_MENU_TICKED;
  }
}
