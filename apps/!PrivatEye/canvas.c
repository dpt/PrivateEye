/* --------------------------------------------------------------------------
 *    Name: canvas.c
 * Purpose: Canvas
 * ----------------------------------------------------------------------- */

// #include "kernel.h"
// #include "swis.h"
//
// #include <assert.h>
// #include <float.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
//
#include "fortify/fortify.h"
//
#include "oslib/types.h"
// #include "oslib/colourtrans.h"
// #include "oslib/filer.h"
// #include "oslib/fileswitch.h"
// #include "oslib/hourglass.h"
// #include "oslib/jpeg.h"
#include "oslib/os.h"
// #include "oslib/osbyte.h"
// #include "oslib/osfile.h"
// #include "oslib/osfind.h"
// #include "oslib/osgbpb.h"
// #include "oslib/osspriteop.h"
#include "oslib/wimp.h"
// #include "oslib/wimpspriteop.h"
//
#include "appengine/types.h"
#include "appengine/datastruct/list.h"
// #include "appengine/app/choices.h"
// #include "appengine/base/bsearch.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
// #include "appengine/base/os.h"
// #include "appengine/base/oserror.h"
// #include "appengine/base/strings.h"
// #include "appengine/dialogues/dcs-quit.h"
// #include "appengine/gadgets/hist.h"
// #include "appengine/gadgets/metadata.h"
// #include "appengine/vdu/screen.h"
// #include "appengine/wimp/dialogue.h"
#include "appengine/wimp/event.h"
// #include "appengine/wimp/filer.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/menu.h"
// #include "appengine/wimp/pointer.h"
//
// #include "actions.h"
// #include "choicesdat.h"
// #include "clipboard.h"
// #include "effects.h"
#include "globals.h"
// #include "iconnames.h"          /* generated */
// #include "info.h"
// #include "keymap.h"
#include "menunames.h"          /* not generated */
#include "privateeye.h"
// #include "rotate.h"
// #include "save.h"
// #include "scale.h"
// #include "tags.h"
// #include "to-spr.h"
// #include "viewer.h"
// #include "zones.h"

#include "canvas.h"

/* ---------------------------------------------------------------------- */

typedef struct canvas_object
{
  enum
  {
    Image
  }
  type;

  os_box bbox;

  union
  {
    struct
    {
      image_t    *image;
      drawable_t *drawable;
    }
    image;
  }
  data;
}
canvas_object;

/* ---------------------------------------------------------------------- */

typedef unsigned int canvas_flags;
#define canvas_flag_UNKNOWN (1u << 0) // for future use

struct canvas
{
  list_t         list; /* a canvas is a linked list node */

  canvas_flags   flags;

  wimp_w         window;

  canvas_object *objects;
  int            nobjects;
  int            maxobjects;
};

/* ---------------------------------------------------------------------- */

static struct
{
  list_t    list_anchor;
  canvas_t *last_canvas; /* last canvas a menu was opened on */
}
LOCALS;

/* ---------------------------------------------------------------------- */

static event_wimp_handler canvas_event_redraw_window_request,
                          canvas_event_close_window_request,
                          canvas_event_mouse_click,
                          canvas_event_key_pressed,
                          canvas_event_menu_selection,
                          canvas_event_scroll_request,
                          canvas_event_lose_caret,
                          canvas_event_gain_caret;

//static event_message_handler canvas_message_palette_change,
//                             canvas_message_mode_change;

/* ----------------------------------------------------------------------- */

// static void canvas_menu_update(void);

/* ----------------------------------------------------------------------- */

static int canvas_event_menu_selection(wimp_event_no event_no,
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
  if (last != GLOBALS.canvas_m)
    return event_NOT_HANDLED;

  /* ... */

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    // canvas_menu_update();
    menu_reopen();
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* set handlers for non-specific events such as menu selections */
static void canvas_register_global_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MENU_SELECTION, canvas_event_menu_selection },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            NULL);
}

/* ----------------------------------------------------------------------- */

error canvas_init(void)
{
  error err;

  /* dependencies */

  err = help_init();
  if (err)
    return err;

  /* handlers */

  canvas_register_global_handlers(1);

  GLOBALS.canvas_w = window_create("canvas");

  /* Menu */

  GLOBALS.canvas_m = menu_create_from_desc(message0("menu.canvas"));

  err = help_add_menu(GLOBALS.canvas_m, "canvas");
  if (err)
    return err;

  /* Keymap */

  //err = viewer_keymap_init();
  //if (err)
  //  return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

void canvas_fin(void)
{
  help_remove_menu(GLOBALS.canvas_m);

  menu_destroy(GLOBALS.canvas_m);

  canvas_register_global_handlers(0);

  help_fin();
}

/* ----------------------------------------------------------------------- */

static int canvas_event_redraw_window_request(wimp_event_no event_no,
                                              wimp_block   *block,
                                              void         *handle)
{
  wimp_draw *redraw;
  osbool     more;

  NOT_USED(event_no);

  redraw = &block->redraw;

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
  {
    /* ... */
  }

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static int canvas_event_close_window_request(wimp_event_no event_no,
                                             wimp_block   *block,
                                             void         *handle)
{
  wimp_close  *close;
  wimp_pointer pointer;

  NOT_USED(event_no);

  close = &block->close;

  wimp_get_pointer_info(&pointer);

  /* ... */

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static int canvas_event_mouse_click(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  wimp_pointer *pointer;

  pointer = &block->pointer;

  // GLOBALS.current_canvas_w = canvas->window;

  if (pointer->buttons & wimp_CLICK_MENU)
  {
    // canvas_menu_update();

    menu_open(GLOBALS.canvas_m, pointer->pos.x - 64, pointer->pos.y);
  }
  else if (pointer->buttons & wimp_CLICK_SELECT)
  {
  }
  else if (pointer->buttons & wimp_CLICK_ADJUST)
  {
  }
  else if (pointer->buttons & wimp_DRAG_SELECT)
  {
  }
  else if (pointer->buttons & wimp_DRAG_ADJUST)
  {
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int canvas_event_key_pressed(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  wimp_key *key;

  NOT_USED(event_no);

  key = &block->key;

  /* ... */

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int canvas_event_scroll_request(wimp_event_no event_no,
                                       wimp_block   *block,
                                       void         *handle)
{
  wimp_scroll *scroll;
  int          d;

  NOT_USED(event_no);
  NOT_USED(handle);

  scroll = &block->scroll;

  switch (scroll->xmin)
  {
  default:
    d = 0;
    break;
  case wimp_SCROLL_PAGE_LEFT:
    d = -(scroll->visible.x1 - scroll->visible.x0);
    break;
  case wimp_SCROLL_PAGE_RIGHT:
    d = scroll->visible.x1 - scroll->visible.x0;
    break;
  case wimp_SCROLL_COLUMN_LEFT:
    d = -GLOBALS.choices.viewer.scroll_x;
    break;
  case wimp_SCROLL_COLUMN_RIGHT:
    d = GLOBALS.choices.viewer.scroll_x;
    break;
  }
  scroll->xscroll += d;

  switch (scroll->ymin)
  {
  default:
    d = 0;
    break;
  case wimp_SCROLL_PAGE_UP:
    d = scroll->visible.y1 - scroll->visible.y0;
    break;
  case wimp_SCROLL_PAGE_DOWN:
    d = -(scroll->visible.y1 - scroll->visible.y0);
    break;
  case wimp_SCROLL_LINE_UP:
    d = GLOBALS.choices.viewer.scroll_y;
    break;
  case wimp_SCROLL_LINE_DOWN:
    d = -GLOBALS.choices.viewer.scroll_y;
    break;
  }
  scroll->yscroll += d;

  wimp_open_window((wimp_open *) scroll);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int canvas_event_lose_caret(wimp_event_no event_no,
                                   wimp_block   *block,
                                   void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int canvas_event_gain_caret(wimp_event_no event_no,
                                   wimp_block   *block,
                                   void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* set handlers for per-window events */
static void canvas_register_window_handlers(int reg, canvas_t *canvas)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, canvas_event_redraw_window_request },
    { wimp_CLOSE_WINDOW_REQUEST,  canvas_event_close_window_request  },
    { wimp_MOUSE_CLICK,           canvas_event_mouse_click           },
    { wimp_KEY_PRESSED,           canvas_event_key_pressed           },
    { wimp_SCROLL_REQUEST,        canvas_event_scroll_request        },
    { wimp_LOSE_CARET,            canvas_event_lose_caret            },
    { wimp_GAIN_CARET,            canvas_event_gain_caret            },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            canvas->window,
                            event_ANY_ICON,
                            canvas);
}

static error canvas_set_window_handlers(canvas_t *canvas)
{
  error err;

  canvas_register_window_handlers(1, canvas);

  err = help_add_window(canvas->window, "canvas");

  return err;
}

static void canvas_unset_window_handlers(canvas_t *canvas)
{
  help_remove_window(canvas->window);

  canvas_register_window_handlers(0, canvas);
}

/* ----------------------------------------------------------------------- */

error canvas_create(canvas_t **new_canvas)
{
  error     err;
  canvas_t *canvas = NULL;
  wimp_w    w      = wimp_ICON_BAR;

  *new_canvas = NULL;

  canvas = malloc(sizeof(*canvas));
  if (canvas == NULL)
    goto NoMem;

  /* Clone ourselves a window */
  w = window_clone(GLOBALS.canvas_w);
  if (w == NULL)
    goto NoMem;

  canvas->window = w;

  /* fill out */

  canvas->flags      = 0;
  canvas->objects    = NULL;
  canvas->nobjects   = 0;
  canvas->maxobjects = 0;

  err = canvas_set_window_handlers(canvas);
  if (err)
    goto Failure;

  list_add_to_head(&LOCALS.list_anchor, &canvas->list);

  *new_canvas = canvas;

  return error_OK;


NoMem:

  err = error_OOM;

  goto Failure;


Failure:

  if (w != wimp_ICON_BAR)
    window_delete_cloned(w);

  free(canvas);

  error_report(err);

  return err;
}

/* ----------------------------------------------------------------------- */

void canvas_destroy(canvas_t *doomed)
{
  if (doomed == NULL)
    return;

  list_remove(&LOCALS.list_anchor, &doomed->list);

  canvas_unset_window_handlers(doomed);

  free(doomed->objects);

  wimp_delete_window(doomed->window);

  free(doomed);
}

/* ----------------------------------------------------------------------- */

void canvas_open(canvas_t *canvas)
{
  window_open(canvas->window);
}

/* ----------------------------------------------------------------------- */

typedef int (canvas_map_callback)(canvas_t *canvas, void *opaque);

/* Call the specified function for every canvas window. */
static void canvas_map(canvas_map_callback *fn, void *opaque)
{
  list_walk(&LOCALS.list_anchor, (list_walk_callback *) fn, opaque);
}

/* ----------------------------------------------------------------------- */

static int close_all_callback(canvas_t *canvas, void *opaque)
{
  NOT_USED(opaque);

  canvas_destroy(canvas);

  return 0;
}

void canvas_close_all(void)
{
  canvas_map(close_all_callback, NULL);
}

/* ----------------------------------------------------------------------- */

int canvas_get_count(void)
{
  /* FIXME: This is not the actual count! It doesn't matter at the moment as
   * this value is only used to determine whether to shade menu entries. */
  return LOCALS.list_anchor.next ? 1 : 0;
}
