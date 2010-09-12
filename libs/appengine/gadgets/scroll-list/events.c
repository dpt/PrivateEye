/* --------------------------------------------------------------------------
 *    Name: events.c
 * Purpose: Scrolling list
 * Version: $Id: events.c,v 1.3 2009-05-20 20:58:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <limits.h>

#include "oslib/colourtrans.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

static int event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  scroll_list *sl;
  wimp_draw   *redraw;
  int          more;

  NOT_USED(event_no);

  sl     = handle;
  redraw = &block->redraw;

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
  {
    os_colour_number c;
    int              x,y;
    int              i;
    int              sel;

    /* Because we wipe the whole work area, this flickers a bit. The
     * alternative is to rely on the client to wipe before drawing.
     * Also we need to wipe the whole window. */

    c = colourtrans_return_colour_number(os_COLOUR_WHITE);
    os_set_colour(os_COLOUR_SET_BG, c);
    os_writec(os_VDU_CLG);

    x = 0;
    y = 0;

    for (i = 0; i < sl->rows; i++) /* draw all rows */
    {
      sel = i == sl->marker;
      y -= sl->leading;
      sl->redraw_lead(redraw, x, y, i, sel);

      sel = i == sl->selection;
      y -= sl->height;
      sl->redraw_elem(redraw, x, y, i, sel);
    }

    sel = i == sl->marker;
    y -= sl->leading;
    sl->redraw_lead(redraw, x, y, i, sel);
  }

  return event_HANDLED;
}

static int event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  scroll_list  *sl;
  wimp_pointer *pointer;

  NOT_USED(event_no);

  sl      = handle;
  pointer = &block->pointer;

  switch (pointer->buttons)
  {
  case wimp_CLICK_SELECT:
  case wimp_CLICK_ADJUST:
  {
    int old_sel;
    int new_sel;

    old_sel = scroll_list__get_selection(sl);
    new_sel = scroll_list__which(sl, pointer);

    if (new_sel >= 0) /* click in range */
    {
      /* clicking with ADJUST toggles the selection */
      if (pointer->buttons & wimp_CLICK_ADJUST && old_sel == new_sel)
        scroll_list__clear_selection(sl);
      else
        scroll_list__set_selection(sl, new_sel);

      send_event(sl, scroll_list__SELECTION_CHANGED);
    }

    break;
  }

  case wimp_DRAG_SELECT:
  {
    int i;

    i = scroll_list__which(sl, pointer);
    if (i >= 0)
    {
      scroll_list__event event;

      event.type  = scroll_list__DRAG;
      event.index = i;
      event.data.drag.pointer = pointer;
      scroll_list__get_bbox(sl, i, &event.data.drag.box);

      sl->event(&event);
    }

    break;
  }
  }

  return event_HANDLED;
}

static int event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  scroll_list *sl;
  wimp_key    *key;

  NOT_USED(event_no);

  sl  = handle;
  key = &block->key;

  switch (key->c)
  {
  case wimp_KEY_HOME:
  case wimp_KEY_COPY:
  case wimp_KEY_DOWN:
  case wimp_KEY_UP:
  case wimp_KEY_PAGE_DOWN:
  case wimp_KEY_PAGE_UP:
  {
    static const struct
    {
      wimp_key_no                 key_no;
      enum { Relative, Absolute } type;
      int                         value;
    }
    map[] =
    {
      { wimp_KEY_UP,        Relative, -1 }, /* note the apparent inversion: up means decrement */
      { wimp_KEY_DOWN,      Relative, +1 },
      { wimp_KEY_PAGE_UP,   Relative, -4 }, // fixed values
      { wimp_KEY_PAGE_DOWN, Relative, +4 },
      { wimp_KEY_HOME,      Absolute, INT_MIN },
      { wimp_KEY_COPY,      Absolute, INT_MAX },
    };

    int old;
    int sel;

    int i;

    for (i = 0; i < NELEMS(map); i++)
      if (map[i].key_no == key->c)
        break;

    /* key will always be found in this case */

    old = sl->selection;

    if (map[i].type == Relative)
      sel = scroll_list__move_selection_relative(sl, map[i].value);
    else /* if (map[i].type == Absolute) */
      sel = scroll_list__move_selection_absolute(sl, map[i].value);

    if (sel != old)
    {
      send_event(sl, scroll_list__SELECTION_CHANGED);
      scroll_list__make_visible(sl, sel);
    }
    break;
  }

  case wimp_KEY_DELETE:
    send_event(sl, scroll_list__DELETE);
    break;

  default:
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void scroll_list__internal_set_handlers(int reg, wimp_w w, scroll_list *sl)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, event_redraw_window_request },
    { wimp_MOUSE_CLICK,           event_mouse_click           },
    { wimp_KEY_PRESSED,           event_key_pressed           },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            w, event_ANY_ICON,
                            sl);
}

