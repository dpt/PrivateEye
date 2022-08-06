/* --------------------------------------------------------------------------
 *    Name: events.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/filer.h"
#include "oslib/font.h"
#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"

#include "datastruct/atom.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/numstr.h"
#include "appengine/base/os.h"
#include "appengine/dialogues/info.h"
#include "appengine/dialogues/name.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"
#include "iconnames.h"
#include "menunames.h"

/* ----------------------------------------------------------------------- */

#define SHOW_BBOXES (0) /* draw bounding boxes (hit areas) around tags */

/* ----------------------------------------------------------------------- */

static event_wimp_handler    tag_cloud_event_null_reason_code,
                             tag_cloud_event_redraw_window_request,
                             tag_cloud_event_open_window_request,
                             tag_cloud_event_pointer_leaving_window,
                             tag_cloud_event_pointer_entering_window,
                             tag_cloud_event_mouse_click,
                             tag_cloud_event_mouse_click_toolbar,
                             tag_cloud_event_key_pressed,
                             tag_cloud_event_menu_selection;

static event_message_handler tag_cloud_message_data_load,
                             tag_cloud_message_mode_change,
                             tag_cloud_message_menus_deleted,
                             tag_cloud_message_font_changed;

/* ----------------------------------------------------------------------- */

void tag_cloud_internal_set_handlers(int reg, tag_cloud *tc)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST,   tag_cloud_event_redraw_window_request   },
    { wimp_OPEN_WINDOW_REQUEST,     tag_cloud_event_open_window_request     },
    { wimp_POINTER_LEAVING_WINDOW,  tag_cloud_event_pointer_leaving_window  },
    { wimp_POINTER_ENTERING_WINDOW, tag_cloud_event_pointer_entering_window },
    { wimp_MOUSE_CLICK,             tag_cloud_event_mouse_click             },
    { wimp_KEY_PRESSED,             tag_cloud_event_key_pressed             },
    { wimp_MENU_SELECTION,          tag_cloud_event_menu_selection          },
  };

  static const event_wimp_handler_spec wimp_handlers_toolbar[] =
  {
    { wimp_MOUSE_CLICK,             tag_cloud_event_mouse_click_toolbar     },
  };

  static const event_message_handler_spec message_handlers_main[] =
  {
    { message_DATA_LOAD,            tag_cloud_message_data_load             },
  };

  static const event_message_handler_spec message_handlers[] =
  {
    { message_MODE_CHANGE,          tag_cloud_message_mode_change           },
    { message_MENUS_DELETED,        tag_cloud_message_menus_deleted         },
    { message_FONT_CHANGED,         tag_cloud_message_font_changed          },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            tc->main_w,
                            event_ANY_ICON,
                            tc);

  if (tc->flags & tag_cloud_FLAG_TOOLBAR)
  {
    event_register_wimp_group(reg,
                              wimp_handlers_toolbar,
                              NELEMS(wimp_handlers_toolbar),
                              tc->toolbar_w,
                              event_ANY_ICON,
                              tc);
  }

  event_register_message_group(reg,
                               message_handlers_main,
                               NELEMS(message_handlers_main),
                               tc->main_w,
                               event_ANY_ICON,
                               tc);

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               tc);
}

static void claim_nulls(int reg, tag_cloud *tc)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, tag_cloud_event_null_reason_code },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            tc->main_w,
                            event_ANY_ICON,
                            tc);
}

/* ----------------------------------------------------------------------- */

static void add_new_tag(dialogue_t *d,
                        const char *tag_name,
                        void       *opaque)
{
  result_t   err;
  tag_cloud *tc = opaque;

  NOT_USED(d);

  if (tc->newtag == NULL)
    return;

  err = tc->newtag(tc, tag_name, strlen(tag_name), tc->opaque);
  result_report(err); /* warn user */

  // FIXME: if ADJUST is clicked to add then the menu stays open, should
  // update it and re-open it
}

static void rename_tag(dialogue_t *d,
                       const char *tag_name,
                       void       *opaque)
{
  result_t   err;
  tag_cloud *tc = opaque;

  NOT_USED(d);

  if (tc->menued_tag_index < 0)
    return;

  if (tc->renametag == NULL)
    return;

  err = tc->renametag(tc,
                      tc->menued_tag_index,
                      tag_name,
                      strlen(tag_name),
                      tc->opaque);
  result_report(err); /* warn user */

  // if (tc->menued_tag_index == -1)
  //   close menu?

  // FIXME: if ADJUST is clicked to rename then the menu stays open, should
  // update it and re-open it
}

static void delete_tag(tag_cloud *tc, int index)
{
  result_t err;

  if (tc->deletetag == NULL)
    return;

  err = tc->deletetag(tc, index, tc->opaque);
  result_report(err); /* warn user */
}

static void rename_fillout(dialogue_t *d, void *opaque)
{
  tag_cloud             *tc = opaque;
  const tag_cloud_entry *entry;

  if (tc->menued_tag_index < 0)
    return;

  entry = &tc->entries[tc->menued_tag_index];

  name_set(d, (const char *) atom_get(tc->dict, entry->atom, NULL));
}

static void tag_info_fillout(dialogue_t *d, void *opaque)
{
  tag_cloud             *tc = opaque;
  const tag_cloud_entry *entry;
  char                   buf[12];
  info_spec_t            specs[2];

  if (tc->menued_tag_index < 0)
    return;

  entry = &tc->entries[tc->menued_tag_index];

  comma_number(entry->count, buf, sizeof(buf));

  specs[0].value = (const char *) atom_get(tc->dict, entry->atom, NULL);
  specs[1].value = buf;

  info_set_info(d, specs, 2);

  info_layout(d);
}

/* ----------------------------------------------------------------------- */

static int tag_cloud_event_null_reason_code(wimp_event_no event_no,
                                            wimp_block   *block,
                                            void         *handle)
{
  static os_coord   last_pos = { -1, -1 };

  tag_cloud        *tc;
  wimp_pointer      pointer;
  wimp_window_state state;
  int               x,y;

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

  tag_cloud_hover(tc, x, y);

  return event_HANDLED;
}

static int tag_cloud_event_redraw_window_request(wimp_event_no event_no,
                                                 wimp_block   *block,
                                                 void         *handle)
{
  wimp_draw *redraw;
  tag_cloud *tc;
  osbool     more;

  NOT_USED(event_no);

  //fprintf(stderr, "tag_cloud_event_redraw_window_request %p\n", handle);

  redraw = &block->redraw;
  tc     = handle;

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
  {
    const font_string_flags flags = font_GIVEN_LENGTH |
                                    font_GIVEN_FONT   |
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

    if (SHOW_BBOXES) /* draw bounding boxes */
    {
      static const os_colour colours[6] =
      {
        os_COLOUR_RED,
        os_COLOUR_GREEN,
        os_COLOUR_BLUE,
        os_COLOUR_CYAN,
        os_COLOUR_MAGENTA,
        os_COLOUR_YELLOW
      };

      int i;

      for (i = 0; i < tc->ntags; i++)
      {
        char buf[4];

        sprintf(buf, "%d", i);

        colourtrans_set_gcol(colours[i % 6], 0, 0, NULL);

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

        os_plot(os_MOVE_TO,
                x + tc->layout.boxes.boxes[i].x0,
                y + tc->layout.boxes.boxes[i].y1);
        os_write0(buf);
      }
    }
  }

  return event_HANDLED;
}

static int tag_cloud_event_open_window_request(wimp_event_no event_no,
                                               wimp_block   *block,
                                               void         *handle)
{
  tag_cloud *tc;
  wimp_open *open;

  NOT_USED(event_no);

  fprintf(stderr, "tag_cloud_event_open_window_request %p\n", handle);

  tc   = handle;
  open = &block->open;

  // here we get the proposed window dimensions

  wimp_open_window(open);

  // open is now the actual window dimensions

  tag_cloud_post_reopen(tc);

  return event_HANDLED;
}

static int tag_cloud_event_pointer_leaving_window(wimp_event_no event_no,
                                                  wimp_block   *block,
                                                  void         *handle)
{
  tag_cloud *tc;

  NOT_USED(event_no);
  NOT_USED(block);

  tc = handle;

  claim_nulls(0, tc);

  tag_cloud_restore_pointer_shape(tc);

  return event_HANDLED;
}

static int tag_cloud_event_pointer_entering_window(wimp_event_no event_no,
                                                   wimp_block   *block,
                                                   void         *handle)
{
  tag_cloud *tc;

  NOT_USED(event_no);
  NOT_USED(block);

  tc = handle;

  claim_nulls(1, tc);

  event_set_earliest(os_read_monotonic_time() + 2);

  return event_HANDLED;
}

static void tag_cloud_menu_update(tag_cloud *tc)
{
  wimp_menu *m;

  /* Main menu */

  m = tc->main_m;

  /* Toolbar entry */

  if (tc->flags & tag_cloud_FLAG_TOOLBAR_NOT_EVER)
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
     * previous call to tag_cloud_menu_update terminated the menu early then
     * ensure that its un-terminated */
    menu_set_menu_flags(m, MAIN_TOOLBAR - 1,
                        wimp_MENU_SEPARATE,
                        wimp_MENU_SEPARATE | wimp_MENU_LAST);

    /* tick the Toolbar entry as required */
    menu_set_menu_flags(m, MAIN_TOOLBAR,
                       (tc->flags & tag_cloud_FLAG_TOOLBAR) ? wimp_MENU_TICKED : 0,
                        wimp_MENU_TICKED);
  }

  /* Display menu */

  m = m->entries[MAIN_DISPLAY].sub_menu;

  menu_range_tick_exclusive(m,
                            DISPLAY_LIST + tag_cloud_get_display(tc),
                            DISPLAY_LIST,
                            DISPLAY_CLOUD);

  menu_range_tick_exclusive(m,
                            DISPLAY_SCALING_OFF + tag_cloud_get_scaling(tc),
                            DISPLAY_SCALING_OFF,
                            DISPLAY_SCALING_ON);

  menu_range_tick_exclusive(m,
                            DISPLAY_SORT_NAME + tag_cloud_get_sort(tc),
                            DISPLAY_SORT_NAME,
                            DISPLAY_SORT_COUNT);

  /* using menu_range_tick_exclusive here even though we're only setting a
   * single item. we map 0..1 to -1..0. */
  menu_range_tick_exclusive(m,
                      DISPLAY_SELECTED_FIRST + tag_cloud_get_order(tc) - 1,
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

  return tag_cloud_hit(tc, x, y);
}

static int tag_cloud_event_mouse_click(wimp_event_no event_no,
                                       wimp_block   *block,
                                       void         *handle)
{
  result_t      err;
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
    if ((tc->flags & tag_cloud_FLAG_SHADED) == 0 && i >= 0)
    {
      tag_cloud_tagfn *tagfn;

      tagfn = tag_cloud_is_highlighted(tc, i) ? tc->detag : tc->tag;

      if (tagfn)
      {
        err = tagfn(tc, i, tc->opaque);
        if (err)
        {
          result_report(err); /* warn user */
          return event_NOT_HANDLED;
        }

        /* kick the pointer to update it's indication. this is cheap as it
         * won't cope if the layout changes. (and it assumes that the state
         * toggles.) */

        tag_cloud_hover_toggle(tc);
      }
    }
    break;

  case wimp_CLICK_MENU:
  {
    const tag_cloud_entry *entry;
    const char            *name;

    tc->menued_tag_index = i; /* keep for later (e.g. Rename, or Delete) */

    /* set tag name */

    entry = &tc->entries[i];

    name = (i >= 0) ? (const char *) atom_get(tc->dict, entry->atom, NULL) : "";

    tc->main_m = menu_create_from_desc(message0("menu.tagcloud"),
                                       name,
                  dialogue_get_window(tag_cloud_get_renametag_dialogue()),
                  dialogue_get_window(tag_cloud_get_taginfo_dialogue()),
                  dialogue_get_window(tag_cloud_get_newtag_dialogue()));

    err = help_add_menu(tc->main_m, "tagcloud");
    if (err)
    {
      result_report(err); /* warn user */
      return event_NOT_HANDLED;
    }

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

    tag_cloud_menu_update(tc);

    /* set handlers in case they're used from the menu */

    name_set_ok_handler(tag_cloud_get_newtag_dialogue(),
                        add_new_tag,
                        tc);

    name_set_ok_handler(tag_cloud_get_renametag_dialogue(),
                        rename_tag,
                        tc);

    dialogue_set_fillout_handler(tag_cloud_get_renametag_dialogue(),
                                 rename_fillout,
                                 tc);

    dialogue_set_fillout_handler(tag_cloud_get_taginfo_dialogue(),
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
  name_set_ok_handler(tag_cloud_get_newtag_dialogue(),
                      add_new_tag,
                      tc);

  dialogue_show(tag_cloud_get_newtag_dialogue());
}

static void show_renametag(tag_cloud *tc)
{
  // this operates on the MENU-clicked-on tag

  if (tc->menued_tag_index < 0)
  {
    beep();
    return;
  }

  name_set_ok_handler(tag_cloud_get_renametag_dialogue(),
                      rename_tag,
                      tc);

  dialogue_set_fillout_handler(tag_cloud_get_renametag_dialogue(),
                               rename_fillout,
                               tc);

  dialogue_show(tag_cloud_get_renametag_dialogue());
}

static void show_taginfo(tag_cloud *tc)
{
  // this operates on the MENU-clicked-on tag

  if (tc->menued_tag_index < 0)
  {
    beep();
    return;
  }

  dialogue_set_fillout_handler(tag_cloud_get_taginfo_dialogue(),
                               tag_info_fillout,
                               tc);

  dialogue_show(tag_cloud_get_taginfo_dialogue());
}

static void send_commit(tag_cloud *tc)
{
  if (tc->event)
    tc->event(tc, tag_cloud_EVENT_COMMIT, tc->opaque);
}

static int tag_cloud_event_mouse_click_toolbar(wimp_event_no event_no,
                                               wimp_block   *block,
                                               void         *handle)
{
  tag_cloud    *tc;
  wimp_pointer *pointer;

  NOT_USED(event_no);

  tc      = handle;
  pointer = &block->pointer;

  if (pointer->buttons == wimp_CLICK_SELECT ||
      pointer->buttons == wimp_CLICK_ADJUST)
  {
    osbool adjust = FALSE;

    switch (pointer->i)
    {
    case TAG_CLOUD_T_B_ADD: /* new tag */
      show_newtag(tc);
      break;

    case TAG_CLOUD_T_B_DISPLIST:
    case TAG_CLOUD_T_B_DISPCLOUD:
      tag_cloud_set_display(tc, (pointer->i == TAG_CLOUD_T_B_DISPLIST) ?
                                 tag_cloud_DISPLAY_TYPE_LIST :
                                 tag_cloud_DISPLAY_TYPE_CLOUD);
      adjust = (pointer->buttons == wimp_CLICK_ADJUST);
      break;

    case TAG_CLOUD_T_B_SORTPOP:   /* sort by count */
    case TAG_CLOUD_T_B_SORTALPHA: /* sort by name  */
      tag_cloud_set_sort(tc, (pointer->i == TAG_CLOUD_T_B_SORTPOP) ?
                              tag_cloud_SORT_TYPE_COUNT :
                              tag_cloud_SORT_TYPE_NAME);
      adjust = (pointer->buttons == wimp_CLICK_ADJUST);
      break;

    case TAG_CLOUD_T_O_SELFIRST:
      tag_cloud_toggle_order(tc);
      break;

    case TAG_CLOUD_T_B_SCALEOFF:
    case TAG_CLOUD_T_B_SCALEON:
      tag_cloud_set_scaling(tc, (pointer->i == TAG_CLOUD_T_B_SCALEOFF) ?
                                 tag_cloud_SCALING_OFF :
                                 tag_cloud_SCALING_ON);
      adjust = (pointer->buttons == wimp_CLICK_ADJUST);
      break;
    }

    if (adjust)
      icon_set_selected(pointer->w, pointer->i, 1);
  }

  return event_HANDLED;
}

static int tag_cloud_event_key_pressed(wimp_event_no event_no,
                                       wimp_block   *block,
                                       void         *handle)
{
  tag_cloud      *tc;
  wimp_key       *key;
  tag_cloud_event event;

  NOT_USED(event_no);

  tc  = handle;
  key = &block->key;

  if (tc->key_handler == NULL)
  {
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  // FIXME: Ought to provide a default here in case the client isn't
  // interested in providing a key handler callback.
  //
  // Could also split this out into a dedicated tag_cloud_process_event which
  // can take events as commands from the client. (Then re-cast the menu
  // handler to use that too). [like action() in PrivateEye].

  event = tc->key_handler(key->c, tc->key_handler_arg);

  switch (event)
  {
  case tag_cloud_EVENT_CLOSE:
    wimp_close_window(tc->main_w);
    break;

  case tag_cloud_EVENT_DISPLAY_LIST:
  case tag_cloud_EVENT_DISPLAY_CLOUD:
    tag_cloud_set_display(tc, (event == tag_cloud_EVENT_DISPLAY_LIST) ?
                               tag_cloud_DISPLAY_TYPE_LIST :
                               tag_cloud_DISPLAY_TYPE_CLOUD);
    break;

  case tag_cloud_EVENT_SORT_BY_NAME:
  case tag_cloud_EVENT_SORT_BY_COUNT:
    tag_cloud_set_sort(tc, (event == tag_cloud_EVENT_SORT_BY_NAME) ?
                            tag_cloud_SORT_TYPE_NAME :
                            tag_cloud_SORT_TYPE_COUNT);
    break;

  case tag_cloud_EVENT_SORT_SELECTED_FIRST:
    tag_cloud_toggle_order(tc);
    break;

  case tag_cloud_EVENT_SCALING_OFF:
  case tag_cloud_EVENT_SCALING_ON:
    tag_cloud_set_scaling(tc, (event == tag_cloud_EVENT_SCALING_OFF) ?
                               tag_cloud_SCALING_OFF :
                               tag_cloud_SCALING_ON);
    break;

  case tag_cloud_EVENT_RENAME:
    show_renametag(tc);
    break;

  case tag_cloud_EVENT_KILL:
    // this operates on the last MENU-clicked-on tag
    if (tc->menued_tag_index < 0)
      beep();
    else
      delete_tag(tc, tc->menued_tag_index);
    break;

  case tag_cloud_EVENT_INFO:
    show_taginfo(tc);
    break;

  case tag_cloud_EVENT_NEW:
    show_newtag(tc);
    break;

  case tag_cloud_EVENT_COMMIT:
    send_commit(tc);
    break;

  default:
    wimp_process_key(key->c);
    break;
  }

  return event_HANDLED;
}

static int tag_cloud_event_menu_selection(wimp_event_no event_no,
                                          wimp_block   *block,
                                          void         *handle)
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

//#define PACK(a,b) (((a) << 8) | (b & 0xFF))

  // static const struct
  // {
  //   unsigned int    items;
  //   tag_cloud_event event;
  // }
  // map[] =
  // {
  //   { PACK(MAIN_DISPLAY, DISPLAY_LIST),           tag_cloud_EVENT_DISPLAY_LIST        },
  //   { PACK(MAIN_DISPLAY, DISPLAY_CLOUD),          tag_cloud_EVENT_DISPLAY_CLOUD       },
  //   { PACK(MAIN_DISPLAY, DISPLAY_SORT_NAME),      tag_cloud_EVENT_SORT_BY_NAME        },
  //   { PACK(MAIN_DISPLAY, DISPLAY_SORT_COUNT),     tag_cloud_EVENT_SORT_BY_COUNT       },
  //   { PACK(MAIN_DISPLAY, DISPLAY_SELECTED_FIRST), tag_cloud_EVENT_SORT_SELECTED_FIRST },
  //   { PACK(MAIN_TAG,     TAG_DELETE),             tag_cloud_EVENT_KILL                },
  //   { PACK(MAIN_COMMIT,  -1),                     tag_cloud_EVENT_COMMIT              },
  //   { PACK(MAIN_TOOLBAR, -1),                     tag_cloud_EVENT_TOOLBAR             }, // doesn't exist
  // };

  switch (selection->items[0])
  {
  case MAIN_DISPLAY:
    switch (selection->items[1])
    {
    case DISPLAY_LIST:
    case DISPLAY_CLOUD:
      tag_cloud_set_display(tc, tag_cloud_DISPLAY_TYPE_LIST + (selection->items[1] - DISPLAY_LIST));
      break;

    case DISPLAY_SCALING_OFF:
    case DISPLAY_SCALING_ON:
      tag_cloud_set_scaling(tc, tag_cloud_SCALING_OFF + (selection->items[1] - DISPLAY_SCALING_OFF));
      break;

    case DISPLAY_SORT_NAME:
    case DISPLAY_SORT_COUNT:
      tag_cloud_set_sort(tc, tag_cloud_SORT_TYPE_NAME + (selection->items[1] - DISPLAY_SORT_NAME));
      break;

    case DISPLAY_SELECTED_FIRST:
      tag_cloud_toggle_order(tc);
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
    tag_cloud_toggle_toolbar(tc);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    tag_cloud_menu_update(tc);
    menu_reopen();
  }
  else
  {
    menu_destroy(tc->main_m);
    tc->main_m = NULL;
  }

  return event_HANDLED;
}

static int tag_cloud_message_data_load(wimp_message *message, void *handle)
{
  result_t                err;
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
    tag_cloud_tagfilefn *tagfilefn;

    tagfilefn = inkey(INKEY_SHIFT) ? tc->detagfile : tc->tagfile;

    if (tagfilefn) /* methods might be NULL */
    {
      err = tagfilefn(tc, xfer->file_name, i, tc->opaque);
      if (err)
      {
        result_report(err); /* warn user */
        return event_NOT_HANDLED;
      }

      // but we need to tag the files specified, not the current file

      // have a different callback to tag/detag a particular file?
    }
  }

  return event_HANDLED;
}

static int tag_cloud_message_mode_change(wimp_message *message, void *handle)
{
  tag_cloud *tc;

  NOT_USED(message);

  tc = handle;

  /* font handles must be closed and re-opened */
  tag_cloud_layout_discard(tc);

  // not NEW_DISPLAY (config doesn't change)
  tc->flags |= tag_cloud_FLAG_NEW_DATA;

  return event_HANDLED;
}

static int tag_cloud_message_menus_deleted(wimp_message *message, void *handle)
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

static int tag_cloud_message_font_changed(wimp_message *message, void *handle)
{
  tag_cloud *tc;

  NOT_USED(message);

  tc = handle;

  tag_cloud_layout_discard(tc);

  // not NEW_DISPLAY (config doesn't change)
  tc->flags |= tag_cloud_FLAG_NEW_DATA;

  tag_cloud_redraw(tc);

  return event_HANDLED;
}
