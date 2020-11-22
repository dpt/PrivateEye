/* --------------------------------------------------------------------------
 *    Name: canvas.c
 * Purpose: Canvas
 * ----------------------------------------------------------------------- */

#ifdef EYE_CANVAS

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/app/wire.h"
#include "appengine/datastruct/list.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/menu.h"

#include "actions.h"
#include "globals.h"
#include "keymap.h"
#include "menunames.h"          /* not generated */
#include "privateeye.h"

#include "canvas.h"

/* ---------------------------------------------------------------------- */

static struct
{
  list_t            list_anchor;    /* linked list of canvases */
  canvas_t         *current_canvas; /* most recent canvas a menu was opened for */
  viewer_keymap_id  keymap_id;
}
LOCALS;

/* ---------------------------------------------------------------------- */

/* key defns */
enum
{
  Canvas_SelectAll,
  Canvas_ClearSelection,
};

/* ---------------------------------------------------------------------- */

static error declare_keymap(void)
{
  /* Keep these sorted by name */
  static const keymap_name_to_action keys[] =
  {
    { "ClearSelection", Canvas_ClearSelection },
    { "SelectAll",      Canvas_SelectAll      },
  };

  return viewer_keymap_add("Canvas",
                           keys,
                           NELEMS(keys),
                           &LOCALS.keymap_id);
}

static error canvas_substrate_callback(const wire_message_t *message,
                                       void                 *opaque)
{
  NOT_USED(opaque);

  switch (message->event)
  {
    case wire_event_DECLARE_KEYMAP:
      return declare_keymap();
  }

  return error_OK;
}

error canvas_substrate_init(void)
{
  error err;

  err = wire_register(0, canvas_substrate_callback, NULL, NULL);
  if (err)
    return err;

  return error_OK;
}

/* ---------------------------------------------------------------------- */

typedef unsigned int canvas_object_flags;
#define canvas_object_flag_SELECTED (1u << 0) // for future use

typedef struct canvas_object
{
  list_t              list; /* a canvas_object is a linked list node */

  canvas_object_flags flags;

  enum
  {
    Image
  }
  type;

  os_box              bbox;

  union
  {
    struct
    {
      image_t        *image;
      drawable_t     *drawable;
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

  list_t         objects_anchor;
};

/* ---------------------------------------------------------------------- */

static event_wimp_handler canvas__event_redraw_window_request,
                          canvas__event_close_window_request,
                          canvas__event_mouse_click,
                          canvas__event_key_pressed,
                          canvas__event_menu_selection,
                          canvas__event_scroll_request,
                          canvas__event_lose_caret,
                          canvas__event_gain_caret;

//static event_message_handler canvas__message_palette_change,
//                             canvas__message_mode_change;

/* ----------------------------------------------------------------------- */

static void canvas__menu_update(void)
{
}

/* ----------------------------------------------------------------------- */

static int canvas__event_menu_selection(wimp_event_no event_no,
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
    canvas__menu_update();
    menu_reopen();
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* set handlers for non-specific events such as menu selections */
static void canvas__register_global_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MENU_SELECTION, canvas__event_menu_selection },
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

  canvas__register_global_handlers(1);

  GLOBALS.canvas_w = window_create("canvas");

  /* Menu */

  GLOBALS.canvas_m = menu_create_from_desc(message0("menu.canvas"));

  err = help_add_menu(GLOBALS.canvas_m, "canvas");
  if (err)
    return err;

  /* Keymap */

  err = viewer_keymap_init();
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

void canvas_fin(void)
{
  viewer_keymap_fin();

  help_remove_menu(GLOBALS.canvas_m);

  menu_destroy(GLOBALS.canvas_m);

  canvas__register_global_handlers(0);

  help_fin();
}

/* ----------------------------------------------------------------------- */

static int canvas__event_redraw_window_request(wimp_event_no event_no,
                                               wimp_block   *block,
                                               void         *handle)
{
  wimp_draw *redraw;
  osbool     more;

  NOT_USED(event_no);
  NOT_USED(handle);

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

static int canvas__event_close_window_request(wimp_event_no event_no,
                                              wimp_block   *block,
                                              void         *handle)
{
  wimp_close  *close;
  wimp_pointer pointer;
  canvas_t    *canvas;

  NOT_USED(event_no);

  close  = &block->close;
  canvas = handle;

  wimp_get_pointer_info(&pointer);

  if (pointer.buttons & wimp_CLICK_ADJUST)
  {
    /* ... open parent stuff goes here ... */
  }

  /* ... query unload stuff goes here ... */

  canvas_destroy(canvas);

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static int canvas__event_mouse_click(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
{
  wimp_pointer *pointer;
  canvas_t     *canvas;

  NOT_USED(event_no);

  pointer = &block->pointer;
  canvas  = handle;

  // GLOBALS.current_canvas_w = canvas->window;

  if (pointer->buttons & wimp_CLICK_MENU)
  {
    LOCALS.current_canvas = canvas;

    canvas__menu_update();

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

static int canvas__event_key_pressed(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
{
  wimp_key *key;
  canvas_t *canvas;
  int       op;

  NOT_USED(event_no);

  key    = &block->key;
  canvas = handle;

  op = viewer_keymap_op(LOCALS.keymap_id, key->c);
  if (op < 0) /* unknown op */
  {
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  switch (op)
  {
    case Canvas_SelectAll:
    case Canvas_ClearSelection:
      break;

    default:
      break;
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int canvas__event_scroll_request(wimp_event_no event_no,
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

static int canvas__event_lose_caret(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int canvas__event_gain_caret(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* set handlers for per-window events */
static void canvas_register_window_handlers(int reg, canvas_t *canvas)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, canvas__event_redraw_window_request },
    { wimp_CLOSE_WINDOW_REQUEST,  canvas__event_close_window_request  },
    { wimp_MOUSE_CLICK,           canvas__event_mouse_click           },
    { wimp_KEY_PRESSED,           canvas__event_key_pressed           },
    { wimp_SCROLL_REQUEST,        canvas__event_scroll_request        },
    { wimp_LOSE_CARET,            canvas__event_lose_caret            },
    { wimp_GAIN_CARET,            canvas__event_gain_caret            },
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

  canvas->flags = 0;

  list_init(&canvas->objects_anchor);

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

  // destroy all objects in the list
  // free(doomed->objects);

  wimp_delete_window(doomed->window);

  free(doomed);
}

/* ----------------------------------------------------------------------- */

void canvas_open(canvas_t *canvas)
{
  window_open(canvas->window);
}

/* ----------------------------------------------------------------------- */

typedef int (canvas__map_callback)(canvas_t *canvas, void *opaque);

/* Call the specified function for every canvas window. */
static void canvas_map(canvas__map_callback *fn, void *opaque)
{
  list_walk(&LOCALS.list_anchor, (list_walk_callback *) fn, opaque);
}

/* ----------------------------------------------------------------------- */

static int canvas__close_all_callback(canvas_t *canvas, void *opaque)
{
  NOT_USED(opaque);

  canvas_destroy(canvas);

  return 0;
}

void canvas_close_all(void)
{
  canvas_map(canvas__close_all_callback, NULL);
}

/* ----------------------------------------------------------------------- */

int canvas_get_count(void)
{
  /* FIXME: This is not the actual count! It doesn't matter at the moment as
   * this value is only used to determine whether to shade menu entries. */
  return LOCALS.list_anchor.next ? 1 : 0;
}

#else

extern int dummy;

#endif /* EYE_CANVAS */
