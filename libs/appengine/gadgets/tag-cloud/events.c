/* --------------------------------------------------------------------------
 *    Name: events.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "oslib/colourtrans.h"
#include "oslib/filer.h"
#include "oslib/font.h"
#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/messages.h"
#include "appengine/base/numstr.h"
#include "appengine/base/os.h"
#include "appengine/datastruct/atom.h"
#include "appengine/dialogues/info.h"
#include "appengine/dialogues/name.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"
#include "iconnames.h"
#include "menunames.h"

/* ----------------------------------------------------------------------- */

static event_wimp_handler    tag_cloud__event_null_reason_code,
                             tag_cloud__event_redraw_window_request,
                             tag_cloud__event_open_window_request,
                             tag_cloud__event_pointer_leaving_window,
                             tag_cloud__event_pointer_entering_window,
                             tag_cloud__event_mouse_click,
                             tag_cloud__event_mouse_click_toolbar,
                             tag_cloud__event_key_pressed,
                             tag_cloud__event_menu_selection;

static event_message_handler tag_cloud__message_data_load,
                             tag_cloud__message_mode_change,
                             tag_cloud__message_menus_deleted,
                             tag_cloud__message_font_changed;

/* ----------------------------------------------------------------------- */

void tag_cloud__internal_set_handlers(int reg, tag_cloud *tc)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST,   tag_cloud__event_redraw_window_request   },
    { wimp_OPEN_WINDOW_REQUEST,     tag_cloud__event_open_window_request     },
    { wimp_POINTER_LEAVING_WINDOW,  tag_cloud__event_pointer_leaving_window  },
    { wimp_POINTER_ENTERING_WINDOW, tag_cloud__event_pointer_entering_window },
    { wimp_MOUSE_CLICK,             tag_cloud__event_mouse_click             },
    { wimp_KEY_PRESSED,             tag_cloud__event_key_pressed             },
    { wimp_MENU_SELECTION,          tag_cloud__event_menu_selection          },
  };

  static const event_wimp_handler_spec wimp_handlers_toolbar[] =
  {
    { wimp_MOUSE_CLICK,             tag_cloud__event_mouse_click_toolbar     },
  };

  static const event_message_handler_spec message_handlers_main[] =
  {
    { message_DATA_LOAD,            tag_cloud__message_data_load             },
  };

  static const event_message_handler_spec message_handlers[] =
  {
    { message_MODE_CHANGE,          tag_cloud__message_mode_change           },
    { message_MENUS_DELETED,        tag_cloud__message_menus_deleted         },
    { message_FONT_CHANGED,         tag_cloud__message_font_changed          },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            tc->main_w, event_ANY_ICON,
                            tc);

  if (tc->flags & tag_cloud__FLAG_TOOLBAR)
  {
    event_register_wimp_group(reg,
                              wimp_handlers_toolbar, NELEMS(wimp_handlers_toolbar),
                              tc->toolbar_w, event_ANY_ICON,
                              tc);
  }

  event_register_message_group(reg,
                               message_handlers_main, NELEMS(message_handlers_main),
                               tc->main_w, event_ANY_ICON,
                               tc);

  event_register_message_group(reg,
                               message_handlers, NELEMS(message_handlers),
                               event_ANY_WINDOW, event_ANY_ICON,
                               tc);
}

static void claim_nulls(int reg, tag_cloud *tc)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, tag_cloud__event_null_reason_code },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            tc->main_w, event_ANY_ICON,
                            tc);
}

/* ----------------------------------------------------------------------- */

static void add_new_tag(dialogue_t *d,
                        const char *tag_name,
                        void       *arg)
{
  tag_cloud *tc = arg;

  NOT_USED(d);

  if (tc->newtag)
    tc->newtag(tc, tag_name, strlen(tag_name), tc->arg);

  // if ADJUST is clicked to add then the menu stays open, should update it and re-open it
}

static void rename_tag(dialogue_t *d,
                       const char *tag_name,
                       void       *arg)
{
  tag_cloud *tc = arg;

  NOT_USED(d);

  if (tc->menued_tag_index < 0)
    return;

  if (tc->renametag)
    tc->renametag(tc, tc->menued_tag_index, tag_name, strlen(tag_name),
                  tc->arg);

  // if (tc->menued_tag_index == -1)
  //   close menu?

  // if ADJUST is clicked to rename then the menu stays open, should update it and re-open it
}

static void delete_tag(tag_cloud *tc, int index)
{
  if (tc->deletetag)
    tc->deletetag(tc, index, tc->arg);
}

static void rename_fillout(dialogue_t *d, void *arg)
{
  tag_cloud *tc = arg;

  if (tc->menued_tag_index < 0)
    return;

  name__set(d, (const char *) atom_get(tc->dict, tc->menued_tag_index, NULL));
}

static void tag_info_fillout(dialogue_t *d, void *arg)
{
  tag_cloud  *tc = arg;
  char        buf[12];
  info_spec_t specs[2];

  if (tc->menued_tag_index < 0)
    return;

  comma_number(tc->entries[tc->menued_tag_index].count, buf, sizeof(buf));

  specs[0].value = (const char *) atom_get(tc->dict, tc->menued_tag_index, NULL);
  specs[1].value = buf;

  info__set_info(d, specs, 2);

  info__layout(d);
}

/* ----------------------------------------------------------------------- */

static int tag_cloud__event_null_reason_code(wimp_event_no event_no, wimp_block *block, void *handle)
{
  static os_coord    last_pos = { -1, -1 };

  tag_cloud         *tc;
  wimp_pointer       pointer;
  wimp_window_state  state;
  int                x,y;

  NOT_USED(event_no);
  NOT_USED(block);

  tc = handle;

  wimp_get_pointer_info(&pointer);

  if (pointer.w != tc->main_w)
    return event_NOT_HANDLED;

  if (pointer.pos.x == last_pos.x && pointer.pos.y == last_pos.y)
    return event_NOT_HANDLED;

  last_pos = pointer.pos;

  state.w = tc->main_w;
  wimp_get_window_state(&state);

  /* highlight the tag we're hovering over */

  x = pointer.pos.x + (state.xscroll - state.visible.x0);
  y = pointer.pos.y + (state.yscroll - state.visible.y1);

  tag_cloud__hover(tc, x, y);

  return event_HANDLED;
}

static int tag_cloud__event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_draw *redraw;
  tag_cloud *tc;
  osbool     more;

  NOT_USED(event_no);

  redraw = &block->redraw;
  tc     = handle;

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
  {
    const font_string_flags flags = font_GIVEN_LENGTH | font_GIVEN_FONT |
                                    font_KERN;
    int x,y;

    x = redraw->box.x0 - redraw->xscroll;
    y = redraw->box.y1 - redraw->yscroll;

    font_paint(tc->layout.font,
               tc->layout.paintstring.string,
               flags,
               x * font_OS_UNIT,
               y * font_OS_UNIT,
               NULL,
               NULL,
               tc->layout.paintstring.used);

    if (0) /* draw bounding boxes */
    {
      int i;

      colourtrans_set_gcol(os_COLOUR_RED, 0, 0, NULL);

      for (i = 0; i < tc->e_used; i++)
      {
        os_plot(os_MOVE_TO,
                x + tc->layout.boxes.boxes[i].x0,
                y + tc->layout.boxes.boxes[i].y0);
        os_plot(os_PLOT_SOLID_EX_END | os_PLOT_TO,
                x + tc->layout.boxes.boxes[i].x0,
                y + tc->layout.boxes.boxes[i].y1);
        os_plot(os_PLOT_SOLID_EX_END | os_PLOT_TO,
                x + tc->layout.boxes.boxes[i].x1,
                y + tc->layout.boxes.boxes[i].y1);
        os_plot(os_PLOT_SOLID_EX_END | os_PLOT_TO,
                x + tc->layout.boxes.boxes[i].x1,
                y + tc->layout.boxes.boxes[i].y0);
        os_plot(os_PLOT_SOLID_EX_END | os_PLOT_TO,
                x + tc->layout.boxes.boxes[i].x0,
                y + tc->layout.boxes.boxes[i].y0);
      }
    }
  }

  return event_HANDLED;
}

static int tag_cloud__event_open_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  tag_cloud *tc;
  wimp_open *open;

  NOT_USED(event_no);

  tc   = handle;
  open = &block->open;

  // here we get the proposed window dimensions

  wimp_open_window(open);

  // open is now the actual window dimensions

  tag_cloud__post_reopen(tc, open);

  return event_HANDLED;
}

static int tag_cloud__event_pointer_leaving_window(wimp_event_no event_no, wimp_block *block, void *handle)
{
  tag_cloud    *tc;
  wimp_leaving *leaving;

  NOT_USED(event_no);

  tc      = handle;
  leaving = &block->leaving;

  claim_nulls(0, tc);

  tag_cloud__restore_pointer_shape(tc);

  return event_HANDLED;
}

static int tag_cloud__event_pointer_entering_window(wimp_event_no event_no, wimp_block *block, void *handle)
{
  tag_cloud     *tc;
  wimp_entering *entering;

  NOT_USED(event_no);

  tc       = handle;
  entering = &block->entering;

  claim_nulls(1, tc);

  event_set_interval(2);

  return event_HANDLED;
}

static void tag_cloud__menu_update(tag_cloud *tc)
{
  wimp_menu *m;

  /* Main menu */

  m = tc->main_m;

  /* Toolbar entry */

  if (tc->flags & tag_cloud__FLAG_TOOLBAR_NOT_EVER)
  {
    /* remove the dotted separator and terminate the menu before the Toolbar
     * entry */
    menu_set_menu_flags(m, MAIN_TOOLBAR - 1,
                        wimp_MENU_LAST,
                        wimp_MENU_SEPARATE | wimp_MENU_LAST);
  }
  else
  {
    /* apply the dotted separator and (possibly unlikely but...) if a
     * previous call to tag_cloud__menu_update terminated the menu early then
     * ensure that its un-terminated */
    menu_set_menu_flags(m, MAIN_TOOLBAR - 1,
                        wimp_MENU_SEPARATE,
                        wimp_MENU_SEPARATE | wimp_MENU_LAST);

    /* tick the Toolbar entry as required */
    menu_set_menu_flags(m, MAIN_TOOLBAR,
                       (tc->flags & tag_cloud__FLAG_TOOLBAR) ? wimp_MENU_TICKED : 0,
                        wimp_MENU_TICKED);

  }

  /* Display menu */

  m = m->entries[MAIN_DISPLAY].sub_menu;

  menu_range_tick_exclusive(m,
                            DISPLAY_LIST + tag_cloud__get_display(tc),
                            DISPLAY_LIST,
                            DISPLAY_CLOUD);

  menu_range_tick_exclusive(m,
                            DISPLAY_SORT_NAME + tag_cloud__get_sort(tc),
                            DISPLAY_SORT_NAME,
                            DISPLAY_SORT_COUNT);

  /* using menu_range_tick_exclusive here even though we're only setting a
   * single item. we map 0..1 to -1..0. */
  menu_range_tick_exclusive(m,
                      DISPLAY_SELECTED_FIRST + tag_cloud__get_order(tc) - 1,
                      DISPLAY_SELECTED_FIRST,
                      DISPLAY_SELECTED_FIRST);
}

static int tag_from_screen_pos(tag_cloud *tc, int x, int y)
{
  wimp_window_state state;

  state.w = tc->main_w;
  wimp_get_window_state(&state);

  x = x + (state.xscroll - state.visible.x0);
  y = y + (state.yscroll - state.visible.y1);

  return tag_cloud__hit(tc, x, y);
}

static int tag_cloud__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  error         err;
  tag_cloud    *tc;
  wimp_pointer *pointer;
  int           i;

  NOT_USED(event_no);

  tc      = handle;
  pointer = &block->pointer;

  i = tag_from_screen_pos(tc, pointer->pos.x, pointer->pos.y);

  switch (pointer->buttons)
  {
  case wimp_CLICK_SELECT:
  case wimp_CLICK_ADJUST:
    /* clicks are disabled when we're shaded */
    if ((tc->flags & tag_cloud__FLAG_SHADED) == 0 && i >= 0)
    {
      tag_cloud__tagfn *tagfn;

      tagfn = tag_cloud__is_highlighted(tc, i) ? tc->detag : tc->tag;

      if (tagfn)
      {
        err = tagfn(tc, i, tc->arg);
        if (err)
          return event_NOT_HANDLED; // handle error

        /* kick the pointer to update it's indication. this is cheap as it
         * won't cope if the layout changes. (and it assumes that the state
         * toggles.) */

        tag_cloud__hover_toggle(tc);
      }
    }
    break;

  case wimp_CLICK_MENU:
  {
    const char *name;

    tc->menued_tag_index = i; /* keep for later (e.g. Rename, or Delete) */

    /* set tag name */

    name = (i >= 0) ? (const char *) atom_get(tc->dict, i, NULL) : "";

    tc->main_m = menu_create_from_desc(message0("menu.tagcloud"),
                                       name,
                  dialogue__get_window(tag_cloud__get_renametag_dialogue()),
                  dialogue__get_window(tag_cloud__get_taginfo_dialogue()),
                  dialogue__get_window(tag_cloud__get_newtag_dialogue()));

    err = help__add_menu(tc->main_m, "tagcloud");
    if (err)
      return event_NOT_HANDLED; // handle error

    /* shade 'Tag 'name'' entry and sub menu when no tag */

    if (i < 0)
    {
      wimp_menu_entry *entry;

      menu_set_menu_flags(tc->main_m, MAIN_TAG,
                          wimp_MENU_SUB_MENU_WHEN_SHADED,
                          wimp_MENU_SUB_MENU_WHEN_SHADED);

      menu_set_icon_flags(tc->main_m, MAIN_TAG,
                           wimp_ICON_SHADED,
                           wimp_ICON_SHADED);

      entry = &tc->main_m->entries[MAIN_TAG];

      menu_shade_all(entry->sub_menu);
    }

    tag_cloud__menu_update(tc);

    /* set handlers in case they're used from the menu */

    name__set_ok_handler(tag_cloud__get_newtag_dialogue(),
                         add_new_tag,
                         tc);

    name__set_ok_handler(tag_cloud__get_renametag_dialogue(),
                         rename_tag,
                         tc);

    dialogue__set_fillout_handler(tag_cloud__get_renametag_dialogue(),
                                  rename_fillout,
                                  tc);

    dialogue__set_fillout_handler(tag_cloud__get_taginfo_dialogue(),
                                  tag_info_fillout,
                                  tc);

    menu_open(tc->main_m, pointer->pos.x - 64, pointer->pos.y);

  }
    break;
  }

  return event_HANDLED;
}

static void show_newtag(tag_cloud *tc)
{
  name__set_ok_handler(tag_cloud__get_newtag_dialogue(),
                       add_new_tag,
                       tc);

  dialogue__show(tag_cloud__get_newtag_dialogue());
}

static void show_renametag(tag_cloud *tc)
{
  // this operates on the MENU-clicked-on tag

  if (tc->menued_tag_index < 0)
  {
    beep();
    return;
  }

  name__set_ok_handler(tag_cloud__get_renametag_dialogue(),
                       rename_tag,
                       tc);

  dialogue__set_fillout_handler(tag_cloud__get_renametag_dialogue(),
                                rename_fillout,
                                tc);

  dialogue__show(tag_cloud__get_renametag_dialogue());
}

static void show_taginfo(tag_cloud *tc)
{
  // this operates on the MENU-clicked-on tag

  if (tc->menued_tag_index < 0)
  {
    beep();
    return;
  }

  dialogue__set_fillout_handler(tag_cloud__get_taginfo_dialogue(),
                                tag_info_fillout,
                                tc);

  dialogue__show(tag_cloud__get_taginfo_dialogue());
}

static void send_commit(tag_cloud *tc)
{
  if (tc->event)
    tc->event(tc, tag_cloud__EVENT_COMMIT, tc->arg);
}

static int tag_cloud__event_mouse_click_toolbar(wimp_event_no event_no, wimp_block *block, void *handle)
{
  tag_cloud    *tc;
  wimp_pointer *pointer;

  NOT_USED(event_no);

  tc      = handle;
  pointer = &block->pointer;

  if (pointer->buttons == wimp_CLICK_SELECT ||
      pointer->buttons == wimp_CLICK_ADJUST)
  {
    switch (pointer->i)
    {
    case TAG_CLOUD_T_B_ADD: /* new tag */
      show_newtag(tc);
      break;

    case TAG_CLOUD_T_B_DISPLIST:
    case TAG_CLOUD_T_B_DISPCLOUD:
      tag_cloud__set_display(tc, (pointer->i == TAG_CLOUD_T_B_DISPLIST) ?
                                  tag_cloud__DISPLAY_TYPE_LIST :
                                  tag_cloud__DISPLAY_TYPE_CLOUD);
      break;

    case TAG_CLOUD_T_B_SORTPOP:   /* sort by count */
    case TAG_CLOUD_T_B_SORTALPHA: /* sort by name  */
      tag_cloud__set_sort(tc, (pointer->i == TAG_CLOUD_T_B_SORTPOP) ?
                               tag_cloud__SORT_TYPE_COUNT :
                               tag_cloud__SORT_TYPE_NAME);
      break;

    case TAG_CLOUD_T_O_SELFIRST:
      tag_cloud__toggle_order(tc);
      break;
    }
  }

  return event_HANDLED;
}

static int tag_cloud__event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  tag_cloud        *tc;
  wimp_key         *key;
  tag_cloud__event  event;

  NOT_USED(event_no);

  tc  = handle;
  key = &block->key;

  if (tc->key_handler == NULL)
  {
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  event = tc->key_handler(key->c, tc->key_handler_arg);

  switch (event)
  {
  case tag_cloud__EVENT_CLOSE:
    wimp_close_window(tc->main_w);
    break;

  case tag_cloud__EVENT_DISPLAY_LIST:
  case tag_cloud__EVENT_DISPLAY_CLOUD:
    tag_cloud__set_display(tc, (event == tag_cloud__EVENT_DISPLAY_LIST) ?
                                tag_cloud__DISPLAY_TYPE_LIST :
                                tag_cloud__DISPLAY_TYPE_CLOUD);
    break;

  case tag_cloud__EVENT_SORT_BY_NAME:
  case tag_cloud__EVENT_SORT_BY_COUNT:
    tag_cloud__set_sort(tc, (event == tag_cloud__EVENT_SORT_BY_NAME) ?
                             tag_cloud__SORT_TYPE_NAME :
                             tag_cloud__SORT_TYPE_COUNT);
    break;

  case tag_cloud__EVENT_SORT_SELECTED_FIRST:
    tag_cloud__toggle_order(tc);
    break;

  case tag_cloud__EVENT_RENAME:
    show_renametag(tc);
    break;

  case tag_cloud__EVENT_KILL:
    // this operates on the last MENU-clicked-on tag
    if (tc->menued_tag_index < 0)
      beep();
    else
      delete_tag(tc, tc->menued_tag_index);
    break;

  case tag_cloud__EVENT_INFO:
    show_taginfo(tc);
    break;

  case tag_cloud__EVENT_NEW:
    show_newtag(tc);
    break;

  case tag_cloud__EVENT_COMMIT:
    send_commit(tc);
    break;

  default:
    wimp_process_key(key->c);
    break;
  }

  return event_HANDLED;
}

static int tag_cloud__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
  tag_cloud      *tc;
  wimp_selection *selection;
  wimp_menu      *last;
  wimp_pointer    p;

  NOT_USED(event_no);

  tc        = handle;
  selection = &block->selection;

  last = menu_last();
  if (last != tc->main_m)
    return event_NOT_HANDLED;

  switch (selection->items[0])
  {
  case MAIN_DISPLAY:
    switch (selection->items[1])
    {
      case DISPLAY_LIST:
      case DISPLAY_CLOUD:
      tag_cloud__set_display(tc, tag_cloud__DISPLAY_TYPE_LIST + (selection->items[1] - DISPLAY_LIST));
      break;

      case DISPLAY_SORT_NAME:
      case DISPLAY_SORT_COUNT:
      tag_cloud__set_sort(tc, tag_cloud__SORT_TYPE_NAME + (selection->items[1] - DISPLAY_SORT_NAME));
      break;

      case DISPLAY_SELECTED_FIRST:
      tag_cloud__toggle_order(tc);
      break;
    }
    break;

  case MAIN_TAG:
    if (selection->items[1] == TAG_DELETE)
      delete_tag(tc, tc->menued_tag_index);
    break;

  case MAIN_COMMIT:
    send_commit(tc);
    break;

  case MAIN_TOOLBAR:
    tag_cloud__toggle_toolbar(tc);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    tag_cloud__menu_update(tc);
    menu_reopen();
  }
  else
  {
    menu_destroy(tc->main_m);
    tc->main_m = NULL;
  }

  return event_HANDLED;
}

static int tag_cloud__message_data_load(wimp_message *message, void *handle)
{
  error                   err;
  wimp_message_data_xfer *xfer;
  tag_cloud              *tc;
  int                     i;

  NOT_USED(message);

  xfer = &message->data.data_xfer;
  tc   = handle;

  if (xfer->w != tc->main_w)
    return event_NOT_HANDLED;

  /* convert pointer pos to tag index */

  i = tag_from_screen_pos(tc, xfer->pos.x, xfer->pos.y);

  if (i < 0)
    return event_NOT_HANDLED; // handle error

  {
    tag_cloud__tagfilefn *tagfilefn;

    tagfilefn = inkey(INKEY_SHIFT) ? tc->detagfile : tc->tagfile;

    if (tagfilefn) /* methods might be NULL */
    {
      err = tagfilefn(tc, xfer->file_name, i, tc->arg);
      if (err)
        return event_NOT_HANDLED; // handle error

      // but we need to tag the files specified, not the current file

      // have a different callback to tag/detag a particular file?
    }
  }

  return event_HANDLED;
}

static int tag_cloud__message_mode_change(wimp_message *message, void *handle)
{
  tag_cloud *tc;

  NOT_USED(message);

  tc = handle;

  /* font handles must be closed and re-opened */
  tag_cloud__layout_discard(tc);

  // not NEW_DISPLAY (config doesn't change)
  tc->flags |= tag_cloud__FLAG_NEW_DATA;

  return event_HANDLED;
}

static int tag_cloud__message_menus_deleted(wimp_message *message, void *handle)
{
  wimp_message_menus_deleted *deleted;
  tag_cloud                  *tc;

  deleted = (wimp_message_menus_deleted *) &message->data;
  tc      = handle;

  if (deleted->menu != tc->main_m)
    return event_NOT_HANDLED;

  menu_destroy(tc->main_m);
  tc->main_m = NULL;

  return event_HANDLED;
}

static int tag_cloud__message_font_changed(wimp_message *message, void *handle)
{
  tag_cloud *tc;

  NOT_USED(message);

  tc = handle;

  tag_cloud__layout_discard(tc);

  // not NEW_DISPLAY (config doesn't change)
  tc->flags |= tag_cloud__FLAG_NEW_DATA;

  tag_cloud__schedule_redraw(tc);

  return event_HANDLED;
}
