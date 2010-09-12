/* --------------------------------------------------------------------------
 *    Name: iconbar.c
 * Purpose: Icon bar icon
 * Version: $Id: iconbar.c,v 1.23 2009-11-29 23:18:36 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/osbyte.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/iconbar.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/messages.h"
#include "appengine/dialogues/prog-info.h"

#include "choicesdat.h"
#include "display.h"
#include "globals.h"
#include "iconbar.h"
#include "iconnames.h"          /* generated */
#include "imagecache.h"
#include "menunames.h"          /* not generated */
#include "quit.h"
#include "tags-search.h"
#include "viewer.h"

/* ---------------------------------------------------------------------- */

static event_wimp_handler icon_bar__event_mouse_click,
                          icon_bar__event_menu_selection;

/* ----------------------------------------------------------------------- */

static void icon_bar__set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK,    icon_bar__event_mouse_click    },
    { wimp_MENU_SELECTION, icon_bar__event_menu_selection },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            wimp_ICON_BAR, event_ANY_ICON,
                            NULL);
}

static dialogue_t *proginfo;

error icon_bar__init(void)
{
  error err;

  /* dependencies */

  err = help__init();
  if (err)
    return err;

  /* prog_info Window */

  proginfo = proginfo__create();

  /* Icon Bar Icon */

  iconbar_create_icon(message0("icon"), wimp_ICON_BAR_RIGHT, 0u);

  err = help__add_window(wimp_ICON_BAR, "iconbar");
  if (err)
    return err;

  /* Icon Bar Menu */

  GLOBALS.iconbar_m = menu_create_from_desc(
                       message0("menu.iconbar"),
                       dialogue__get_window(proginfo));

  err = help__add_menu(GLOBALS.iconbar_m, "iconbar");
  if (err)
    return err;

  icon_bar__set_handlers(1);

  return error_OK;
}

void icon_bar__fin(void)
{
  icon_bar__set_handlers(0);

  help__remove_menu(GLOBALS.iconbar_m);

  menu_destroy(GLOBALS.iconbar_m);

  help__remove_window(wimp_ICON_BAR);

  proginfo__destroy(proginfo);

  help__fin();
}

/* ----------------------------------------------------------------------- */

static void icon_bar__menu_update(void)
{
  /* Shade the "Close all" entry if there are no images open */
  menu_set_icon_flags(GLOBALS.iconbar_m,
                      ICONBAR_CLOSE,
                     (viewer_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);

  /* Shade the "Empty cache" entry if the cache is already empty */
  menu_set_icon_flags(GLOBALS.iconbar_m,
                      ICONBAR_EMPTYCACHE,
                     (imagecache_get_count() > 0) ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
}

static int icon_bar__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & wimp_CLICK_MENU)
  {
    icon_bar__menu_update();

    /* AppEngine could do with a menu_open_iconbar which calculates the
     * appropriate height to open at */
    menu_open(GLOBALS.iconbar_m,
              pointer->pos.x - 64,
              96 + wimp_MENU_ITEM_HEIGHT * ICONBAR__LIMIT);
  }

  return event_HANDLED;
}

static int icon_bar__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_selection *selection;
  wimp_menu      *last;
  wimp_pointer    p;

  NOT_USED(event_no);
  NOT_USED(handle);

  selection = &block->selection;

  last = menu_last();
  if (last != GLOBALS.iconbar_m)
    return event_NOT_HANDLED;

  switch (selection->items[0])
  {
  case ICONBAR_CLOSE:
    /* Close all */
    if (can_quit())
      viewer_close_all();
    break;

  case ICONBAR_EMPTYCACHE:
    imagecache_empty();
    break;

  case ICONBAR_SEARCHTAGS:
#ifdef EYE_TAGS
    tags_search__open();
#endif
    break;

  case ICONBAR_CHOICES:
    /* Choices... */
    choices_open(&privateeye_choices);
    break;

  case ICONBAR_QUIT:
    /* Quit */
    if (can_quit())
      GLOBALS.flags |= Flag_Quit;
    break;
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    icon_bar__menu_update();
    menu_reopen();
  }

  return event_HANDLED;
}
