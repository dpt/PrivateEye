
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"

#include "appengine/wimp/menu.h"

/* TODO: adjust the menu width for the title and each subsequent entry */

static int menu_title_is_indirected = 0;

wimp_menu *menu_create(const char *title_token)
{
  const char *entry_token;
  char       *wildcarded_token;
  int         index;
  int         entries;
  wimp_menu  *menu_base;
  wimp_menu  *menu_ptr;

  /* construct a wildcarded version of the token for use when enumerating */

  wildcarded_token = malloc(strlen(title_token) + 3);
  if (wildcarded_token == NULL)
    goto oom;

  strcpy(wildcarded_token, title_token);
  strcat(wildcarded_token, ".*");

  /* enumerate tokens, working out the required memory */

  entries     = 0;
  index       = 0;
  entry_token = messages_enumerate(wildcarded_token, &index);
  while (entry_token)
  {
    entries++;
    entry_token = messages_enumerate(wildcarded_token, &index);
  };

  /* allocate space for the menu structure */

  menu_base = malloc(sizeof(wimp_menu) + sizeof(wimp_menu_entry) * (entries - 1));
  if (menu_base == NULL)
    goto oom;

  /* build menu */

  menu_ptr = menu_base;

  /* title */

  menu_title(&menu_ptr, title_token);

  /* enumerate tokens, adding menu entries */

  index = 0;
  entry_token = messages_enumerate(wildcarded_token, &index);
  while (entry_token)
  {
    menu_entry(&menu_ptr, entry_token);
    entry_token = messages_enumerate(wildcarded_token, &index);
  };

  /* terminate structure (set last entry's flags to finish) */

  menu_set_menu_flags(menu_base, entries - 1, wimp_MENU_LAST, wimp_MENU_LAST);

  free(wildcarded_token);

  return menu_base;


oom:

  error__fatal_oom();

  return NULL; /* never reached */
}

void menu_title(wimp_menu **block, const char *token)
{
  const char *text;
  int         text_len;
  wimp_menu  *menu;

  menu = *block;

  text = message(token);
  text_len = strlen(text);
  if (text_len < 12)
  {
    /* make menu title */
    strcpy(menu->title_data.text, text);
  }
  else
  {
    /* indirected */
    menu->title_data.indirected_text.text      = str_dup(text);
    if (menu->title_data.indirected_text.text == NULL)
      goto oom;

    /* validation and text_size fields are reserved (don't exist in OSLib) */

    /* first menu entry should set flags bit 8 */
    menu_title_is_indirected = 1;
  }

  menu->title_fg = 7; /* black */
  menu->title_bg = 2; /* grey */
  menu->work_fg  = 7; /* black */
  menu->work_bg  = 0; /* white */
  menu->width    = (text_len + 2) << 4;
  menu->height   = 44;
  menu->gap      = 0;

  *((char **) block) += sizeof(wimp_menu) - sizeof(wimp_menu_entry);

  return;


oom:

  error__fatal_oom();
}

void menu_entry(wimp_menu **block, const char *token)
{
  const char      *text;
  int              text_len;
  wimp_menu_entry *menu_entry;

  menu_entry = (wimp_menu_entry *) *block;

  text = message(token);
  text_len = strlen(text);
  if (text_len < 12)
  {
    /* ordinary */

    menu_entry->menu_flags = menu_title_is_indirected << 8;
    menu_entry->sub_menu   = NULL;
    menu_entry->icon_flags = 0x07000021; /* non-indirected */
    strcpy(menu_entry->data.text, text);
  }
  else
  {
    /* indirected */

    menu_entry->menu_flags                = menu_title_is_indirected << 8;
    menu_entry->sub_menu                  = NULL;
    menu_entry->icon_flags                = 0x07000121; /* indirected */
    menu_entry->data.indirected_text.text = str_dup(text);
    if (menu_entry->data.indirected_text.text == NULL)
      goto oom;

    menu_entry->data.indirected_text.validation = NULL;
    menu_entry->data.indirected_text.size       = text_len + 1;
  }

  menu_title_is_indirected = 0;

  /*if ((text_len + 2) * 16> menu width)
  {
    menu width = (text_len + 2) * 16;
  }*/

  *((char **) block) += sizeof(wimp_menu_entry);

  return;


oom:

  error__fatal_oom();
}
