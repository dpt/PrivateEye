
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"

#include "appengine/wimp/menu.h"

/* ----------------------------------------------------------------------- */

#define MENUWIDTH(x) (((x) + 1) * 16)

/* ----------------------------------------------------------------------- */

static int menu_width;
static int menu_title_is_indirected;

/* ----------------------------------------------------------------------- */

wimp_menu *menu_create(const char *title_token)
{
  char       *wildcarded_token;
  int         entries;
  int         index;
  wimp_menu  *menu_base;
  wimp_menu  *menu_ptr;
  const char *entry_token;

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
  }

  /* allocate space for the menu structure */

  menu_base = malloc(sizeof(wimp_menu) + sizeof(wimp_menu_entry) * (entries - 1));
  if (menu_base == NULL)
    goto oom;

  /* build menu */

  menu_width               = 0;
  menu_title_is_indirected = 0;

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
  }

  /* terminate structure (set last entry's flags to finish) */

  menu_set_menu_flags(menu_base, entries - 1, wimp_MENU_LAST, wimp_MENU_LAST);

  menu_base->width = menu_width;

  free(wildcarded_token);

  return menu_base;


oom:

  error_fatal_oom();

  return NULL; /* never reached */
}

void menu_title(wimp_menu **block, const char *token)
{
  wimp_menu  *menu;
  const char *text;
  int         text_len;

  menu = *block;

  text = message(token);
  text_len = strlen(text);
  if (text_len < 12)
  {
    strcpy(menu->title_data.text, text);
  }
  else
  {
    /* indirected */
    menu->title_data.indirected_text.text = str_dup(text);
    if (menu->title_data.indirected_text.text == NULL)
      goto oom;

    /* title_data.validation and text_size fields are reserved (don't exist in OSLib) */

    /* first menu entry should set flags bit 8 */
    menu_title_is_indirected = 1;
  }

  menu->title_fg = wimp_COLOUR_BLACK;
  menu->title_bg = wimp_COLOUR_LIGHT_GREY;
  menu->work_fg  = wimp_COLOUR_BLACK;
  menu->work_bg  = wimp_COLOUR_WHITE;
  menu->width    = 0; /* filled out at the end */
  menu->height   = 44;
  menu->gap      = 0;

  menu_width = MENUWIDTH(text_len);

  *((char **) block) += sizeof(wimp_menu) - sizeof(wimp_menu_entry);

  return;


oom:

  error_fatal_oom();
}

void menu_entry(wimp_menu **block, const char *token)
{
  const wimp_icon_flags icon_flags = (wimp_COLOUR_BLACK << wimp_ICON_FG_COLOUR_SHIFT) |
                                      wimp_ICON_FILLED |
                                      wimp_ICON_TEXT;

  wimp_menu_entry *menu_entry;
  const char      *text;
  int              text_len;

  menu_entry = (wimp_menu_entry *) *block;

  text = message(token);
  text_len = strlen(text);

  menu_entry->menu_flags = menu_title_is_indirected << 8;
  menu_entry->sub_menu   = NULL;

  if (text_len < 12)
  {
    /* ordinary */
    menu_entry->icon_flags = icon_flags;
    strcpy(menu_entry->data.text, text);
  }
  else
  {
    /* indirected */
    menu_entry->icon_flags                = icon_flags | wimp_ICON_INDIRECTED;
    menu_entry->data.indirected_text.text = str_dup(text);
    if (menu_entry->data.indirected_text.text == NULL)
      goto oom;

    menu_entry->data.indirected_text.validation = NULL;
    menu_entry->data.indirected_text.size       = text_len + 1;
  }

  menu_title_is_indirected = 0;

  if (MENUWIDTH(text_len) > menu_width)
    menu_width = MENUWIDTH(text_len);

  *((char **) block) += sizeof(wimp_menu_entry);

  return;


oom:

  error_fatal_oom();
}
