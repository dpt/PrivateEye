/* $Id: destroy.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/menu.h"

void menu_destroy(wimp_menu *menu)
{
  const wimp_menu_entry *entry;

  /* walk menu data, freeing all indirected items */

  entry = menu->entries;

  if (entry->menu_flags & wimp_MENU_TITLE_INDIRECTED)
    free(menu->title_data.indirected_text.text);

  for (;;)
  {
    /* handles with a their bottom bit set are assumed to be window handles,
     * and are ignored. otherwise they're taken to be pointers to menu
     * structures which will be freed. */

    if ((unsigned int) entry->sub_menu >= 0x8000 &&
        ((unsigned int) entry->sub_menu & 1) == 0)
      menu_destroy(entry->sub_menu);

    if (entry->icon_flags & wimp_ICON_INDIRECTED)
      free(entry->data.indirected_text.text);

    if (entry->menu_flags & wimp_MENU_LAST)
      break;

    entry++;
  }

  free(menu);
}
