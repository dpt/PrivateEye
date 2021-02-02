/* --------------------------------------------------------------------------
 *    Name: iconbar.c
 * Purpose: Standard icon bar icon
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/messages.h"
#include "appengine/dialogues/info.h"
#include "appengine/dialogues/prog-info.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/iconbar.h"
#include "appengine/wimp/menu.h"

#include "appengine/gadgets/iconbar.h"

static struct
{
  dialogue_t                *proginfo;
  wimp_menu                 *iconbar_m;
  icon_bar_menu_pointerfn   *ptrfn;
  icon_bar_menu_selectionfn *selfn;
  icon_bar_menu_updatefn    *updfn;
  void                      *opaque;
  int                        menu_height; /* in entries */
}
LOCALS;

/* ---------------------------------------------------------------------- */

static event_wimp_handler icon_bar_event_mouse_click,
                          icon_bar_event_menu_selection;

/* ----------------------------------------------------------------------- */

static void icon_bar_internal_set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK,    icon_bar_event_mouse_click    },
    { wimp_MENU_SELECTION, icon_bar_event_menu_selection },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            wimp_ICON_BAR,
                            event_ANY_ICON,
                            NULL);
}

result_t icon_bar_init(void)
{
  result_t err;

  /* dependencies */

  err = help_init();
  if (err)
    return err;

  /* prog_info Window */

  LOCALS.proginfo = proginfo_create();

  /* Icon Bar Icon */

  iconbar_create_icon(message0("icon"), wimp_ICON_BAR_RIGHT, 0u);

  err = help_add_window(wimp_ICON_BAR, "iconbar");
  if (err)
    return err;

  /* Icon Bar Menu */

  LOCALS.iconbar_m = menu_create_from_desc(message0("menu.iconbar"),
                                           dialogue_get_window(LOCALS.proginfo));

  LOCALS.menu_height = menu_count(LOCALS.iconbar_m);

  err = help_add_menu(LOCALS.iconbar_m, "iconbar");
  if (err)
    return err;

  icon_bar_internal_set_handlers(1);

  return result_OK;
}

void icon_bar_fin(void)
{
  icon_bar_internal_set_handlers(0);

  help_remove_menu(LOCALS.iconbar_m);

  menu_destroy(LOCALS.iconbar_m);

  help_remove_window(wimp_ICON_BAR);

  proginfo_destroy(LOCALS.proginfo);

  help_fin();
}

void icon_bar_set_handlers(icon_bar_menu_pointerfn   *pointer,
                           icon_bar_menu_selectionfn *select,
                           icon_bar_menu_updatefn    *update,
                           void                      *opaque)
{
  LOCALS.ptrfn  = pointer;
  LOCALS.selfn  = select;
  LOCALS.updfn  = update;
  LOCALS.opaque = opaque;
}

/* ----------------------------------------------------------------------- */

static void icon_bar_menu_update(void)
{
  if (LOCALS.updfn)
    LOCALS.updfn(LOCALS.iconbar_m, LOCALS.opaque);
}

static int icon_bar_event_mouse_click(wimp_event_no event_no,
                                      wimp_block   *block,
                                      void         *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & wimp_CLICK_MENU)
  {
    icon_bar_menu_update();

    menu_open(LOCALS.iconbar_m,
              pointer->pos.x - 64,
              96 + wimp_MENU_ITEM_HEIGHT * LOCALS.menu_height);
  }
  else
  {
    if (LOCALS.ptrfn)
      LOCALS.ptrfn(&block->pointer, LOCALS.opaque);
  }

  return event_HANDLED;
}

static int icon_bar_event_menu_selection(wimp_event_no event_no,
                                         wimp_block   *block,
                                         void         *handle)
{
  wimp_selection *selection;
  wimp_menu      *last;
  wimp_pointer    p;

  NOT_USED(event_no);
  NOT_USED(handle);

  selection = &block->selection;

  last = menu_last();
  if (last != LOCALS.iconbar_m)
    return event_NOT_HANDLED;

  if (LOCALS.selfn)
    LOCALS.selfn(selection, LOCALS.opaque);

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    icon_bar_menu_update();
    menu_reopen();
  }

  return event_HANDLED;
}
