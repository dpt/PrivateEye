/* --------------------------------------------------------------------------
 *    Name: filerwin.c
 * Purpose: Filer-style grid window
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"
#include "oslib/wimpspriteop.h"

#include "appengine/types.h"
#include "appengine/geom/box.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/base/os.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/window.h"
#include "appengine/datastruct/bitvec.h"

#include "appengine/gadgets/filerwin.h"

#define drag_type_SELECTION_SELECT 1
#define drag_type_SELECTION_ADJUST 2

struct filerwin
{
  wimp_w               w;
  filerwin_redrawfn  *redraw;
  filerwin_closefn   *close;
  filerwin_pointerfn *pointer;
  char                *title_text;

  // told
  int                  object_width, object_height;
  int                  hpad, vpad;
  int                  nobjects;
  filerwin_mode       mode;
  filerwin_sort       sort;

  void                *arg;

  // computed
  int                  last_width, last_height;
  int                  columns, rows;

  // foo
  int                  drag_type;

  bitvec_t            *selection;
};

/* ---------------------------------------------------------------------- */

static event_wimp_handler filerwin_event_redraw_window_request,
                          filerwin_event_open_window_request,
                          filerwin_event_close_window_request,
                          filerwin_event_mouse_click,
                          filerwin_event_user_drag_box,
                          filerwin_event_key_pressed;

/* ---------------------------------------------------------------------- */

static
void filerwin_internal_set_handlers(int reg, filerwin *fw)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, filerwin_event_redraw_window_request },
    { wimp_OPEN_WINDOW_REQUEST,   filerwin_event_open_window_request   },
    { wimp_CLOSE_WINDOW_REQUEST,  filerwin_event_close_window_request  },
    { wimp_MOUSE_CLICK,           filerwin_event_mouse_click           },
    { wimp_USER_DRAG_BOX,         filerwin_event_user_drag_box         },
    { wimp_KEY_PRESSED,           filerwin_event_key_pressed           },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            fw->w, event_ANY_ICON,
                            fw);
}

/* ---------------------------------------------------------------------- */

static const wimp_window wdef =
{
  { 0, 0, 1024, 512 }, /* visible */
  0, 0,
  wimp_TOP,
  wimp_WINDOW_MOVEABLE | wimp_WINDOW_BACK_ICON | wimp_WINDOW_CLOSE_ICON | wimp_WINDOW_TITLE_ICON | wimp_WINDOW_TOGGLE_ICON | wimp_WINDOW_VSCROLL | wimp_WINDOW_SIZE_ICON | wimp_WINDOW_NEW_FORMAT,
  wimp_COLOUR_BLACK,
  wimp_COLOUR_LIGHT_GREY,
  wimp_COLOUR_BLACK,
  wimp_COLOUR_VERY_LIGHT_GREY,
  wimp_COLOUR_MID_LIGHT_GREY,
  wimp_COLOUR_VERY_LIGHT_GREY,
  wimp_COLOUR_CREAM,
  0,
  { 0, -512, 1024, 0 }, /* extent */
  wimp_ICON_TEXT | wimp_ICON_HCENTRED | wimp_ICON_VCENTRED | wimp_ICON_INDIRECTED,
  wimp_BUTTON_DOUBLE_CLICK_DRAG << wimp_ICON_BUTTON_TYPE_SHIFT,
  wimpspriteop_AREA,
  4, 0,
  { .indirected_text = { '\0', '\0', 0 } },
  0,
};

/* ----------------------------------------------------------------------- */

typedef error (mapfn)(filerwin *fw, int x, int y, int c, unsigned int flags, void *arg);

static error map(filerwin *fw, mapfn *fn, int x, int y,
                 const os_box *test, void *arg)
{
  error err;
  int   c;
  int   yr;
  int   yy;

  c = 0;

  yr = y;

  for (yy = 0; yy < fw->rows; yy++)
  {
    int xr;
    int xx;

    xr = x + fw->hpad;
    yr = yr - fw->vpad - fw->object_height;

    for (xx = 0; xx < fw->columns; xx++)
    {
      os_box box;

      if (c >= fw->nobjects)
        break;

      box.x0 = xr;
      box.y0 = yr;
      box.x1 = box.x0 + fw->object_width;
      box.y1 = box.y0 + fw->object_height;

      if (!test || (test && box_intersects(&box, test)))
      {
        err = fn(fw, xr, yr, c, bitvec_get(fw->selection, c), arg);
        if (err)
          return err;
      }

      xr += fw->object_width + fw->hpad;

      c++;
    }
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static error redraw_bobs(filerwin *fw, int x, int y, int c, unsigned int flags, void *arg)
{
  wimp_draw *redraw;

  redraw = arg;

  fw->redraw(redraw, x, y, c, flags & 1 /* flags -> selection */, fw->arg);

  return error_OK;
}

static int filerwin_event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  error      err;
  wimp_draw *redraw;
  filerwin  *fw;
  osbool     more;

  NOT_USED(event_no);

  redraw = &block->redraw;
  fw     = handle;

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
  {
    int x,y;

    x = redraw->box.x0 - redraw->xscroll;
    y = redraw->box.y1 - redraw->yscroll;

    err = map(fw, redraw_bobs, x, y, &redraw->clip, redraw);
    if (err)
      return event_HANDLED;
  }

  return event_HANDLED;
}

static int layout(filerwin *fw, int width, int height)
{
  int sw, sh;
  int columns, rows;
  int maxcolumns, maxwidth;

  NOT_USED(height);

  read_screen_dimensions(&sw, &sh);

  // set new y
  // enforce a maximum width constrained by max number of objects columns
  //   or is it the screen width?

  columns = (width - fw->hpad) / (fw->object_width + fw->hpad);
  columns = CLAMP(columns, 1, fw->nobjects);
  rows    = (fw->nobjects + columns - 1) / columns; /* round up */

  maxcolumns = (sw - fw->hpad) / (fw->object_width + fw->hpad);
  maxcolumns = CLAMP(maxcolumns, 1, fw->nobjects);
  maxwidth   = fw->hpad + maxcolumns * (fw->object_width + fw->hpad);

  /* if the rows/columns have changed */
  if (columns != fw->columns || rows != fw->rows)
  {
    os_box box;

    //fprintf(stderr, "%d %d, %d %d\n", columns, fw->columns, rows, fw->rows);

    box.x0 = 0;
    box.y0 = - fw->vpad - rows * (fw->object_height + fw->vpad);
    box.x1 = maxwidth;
    box.y1 = 0;
    wimp_set_extent(fw->w, &box);

    wimp_force_redraw(fw->w, box.x0, box.y0, box.x1, box.y1);

    fw->columns = columns;
    fw->rows    = rows;
  }

  return 0;
}

static int filerwin_event_open_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_open *open;
  filerwin  *fw;

  NOT_USED(event_no);

  open = &block->open;
  fw   = handle;

  {
    int width, height;

    width  = open->visible.x1 - open->visible.x0;
    height = open->visible.y1 - open->visible.y0;

    /* if the window has changed size */
    if (width != fw->last_width || height != fw->last_height)
    {
      layout(fw, width, height);
      fw->last_width  = width;
      fw->last_height = height;
    }
  }

  wimp_open_window(open);

  return event_HANDLED;
}

static int filerwin_event_close_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_close  *close;
  filerwin    *fw;
  wimp_pointer pointer;

  NOT_USED(event_no);

  close = &block->close;
  fw    = handle;

  wimp_get_pointer_info(&pointer);

  /* Note that we might be entered if another part of the program has
   * faked a close event, in which case the pointer may or may not have
   * buttons held.
   */

  if (pointer.buttons & wimp_CLICK_ADJUST)
  {
#if 0
    filer_open_dir(viewer->drawable->image->file_name);
#endif

    if (inkey(INKEY_SHIFT))
      return event_HANDLED;
  }

#if 0
  if (viewer_query_unload(viewer))
  {
    viewer_unload(viewer);
    viewer_destroy(viewer);
  }
#endif

  fw->close(close, fw->arg);

  return event_HANDLED;
}

static int filerwin_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  filerwin     *fw;

  NOT_USED(event_no);

  pointer = &block->pointer;
  fw      = handle;

  switch (pointer->buttons)
  {
  case wimp_DOUBLE_SELECT:
  case wimp_DOUBLE_ADJUST:
    // SELECT: 'run' object, then deselect it
    // ADJUST:  'run' object, then close viewer
    break;

  case wimp_SINGLE_SELECT:
  case wimp_SINGLE_ADJUST:
    // SELECT: select clicked object
    // ADJUST: toggle clicked object
    break;

  case wimp_DRAG_SELECT:
  case wimp_DRAG_ADJUST:
  {
    wimp_window_info info;
    wimp_drag        drag;
    wimp_version_no  version;

    // dragged over an object? ...
    //  SELECT: select it, start drag op
    //  ADJUST: toggle it, then select it, start drag op
    // ie. start drag op always selects

    fw->drag_type = (pointer->buttons == wimp_DRAG_SELECT) ?
                     drag_type_SELECTION_SELECT : drag_type_SELECTION_ADJUST;

    info.w = fw->w;
    wimp_get_window_info_header_only(&info);

    drag.w          = fw->w;
    drag.type       = wimp_DRAG_USER_RUBBER;
    drag.initial.x0 = pointer->pos.x;
    drag.initial.y0 = pointer->pos.y;
    drag.initial.x1 = pointer->pos.x;
    drag.initial.y1 = pointer->pos.y;
    drag.bbox.x0    = info.visible.x0 - info.xscroll;
    drag.bbox.y0    = 0x8000;
    drag.bbox.x1    = info.visible.x1 - info.xscroll;
    drag.bbox.y1    = 0x7fff;
    drag.handle     = NULL;
    drag.draw       =
    drag.undraw     =
    drag.redraw     = NULL;

    xwimpreadsysinfo_version(&version);

    if (version >= wimp_VERSION_RO40)
    {
      wimp_auto_scroll_info scrinfo;

      wimp_drag_box_with_flags(&drag, wimp_DRAG_BOX_KEEP_IN_LINE |
                                      wimp_DRAG_BOX_CLIP);

      /* The following settings are Filer compatible. */

      scrinfo.w                   = fw->w;
      scrinfo.pause_zone_sizes.x0 =
      scrinfo.pause_zone_sizes.y0 =
      scrinfo.pause_zone_sizes.x1 =
      scrinfo.pause_zone_sizes.y1 = 12;
      scrinfo.pause_duration      =  0;
      scrinfo.state_change        = wimp_AUTO_SCROLL_DEFAULT_HANDLER;
      scrinfo.handle              = NULL;

      wimp_auto_scroll(wimp_AUTO_SCROLL_ENABLE_VERTICAL,
                      &scrinfo);
    }
    else
    {
      wimp_drag_box(&drag);
    }
  }
    break;

  case wimp_CLICK_MENU:
    // selection:
    // if nothing selected, highlight the entry under the pointer, if one
    // if existing selection, preserve it

    fw->pointer(pointer, fw->arg);
    break;
  }

  return event_HANDLED;
}

static void index_to_area(filerwin *fw, int index, os_box *box)
{
  int iq,ir;
  int x,y;

  iq = index / fw->columns;
  ir = index % fw->columns;

  x = fw->hpad + ir * (fw->object_width + fw->hpad);
  y = iq * (- fw->vpad - fw->object_height);

  box->x0 = x;
  box->y0 = y - fw->vpad - fw->object_height;
  box->x1 = x + fw->object_width;
  box->y1 = y;
}

static error select_bobs(filerwin *fw, int x, int y, int c, unsigned int flags, void *arg)
{
  os_box b;

  NOT_USED(x);
  NOT_USED(y);
  NOT_USED(flags);
  NOT_USED(arg);

  bitvec_set(fw->selection, c);

  index_to_area(fw, c, &b);
  wimp_force_redraw(fw->w, b.x0, b.y0, b.x1, b.y1);

  return error_OK;
}

static error adjust_bobs(filerwin *fw, int x, int y, int c, unsigned int flags, void *arg)
{
  os_box b;

  NOT_USED(x);
  NOT_USED(y);
  NOT_USED(flags);
  NOT_USED(arg);

  bitvec_toggle(fw->selection, c);

  index_to_area(fw, c, &b);
  wimp_force_redraw(fw->w, b.x0, b.y0, b.x1, b.y1);

  return error_OK;
}

// hoist out to vdu lib
static void screen_box_to_workarea(os_box *b, const wimp_window_state *state)
{
  int x,y;
  int temp;

  x = - state->visible.x0 + state->xscroll;
  y = - state->visible.y1 + state->yscroll;

  b->x0 += x;
  b->y0 += y;
  b->x1 += x;
  b->y1 += y;

  if (b->x0 > b->x1) { temp = b->x1; b->x1 = b->x0; b->x0 = temp; }
  if (b->y0 > b->y1) { temp = b->y1; b->y1 = b->y0; b->y0 = temp; }
}

static int filerwin_event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_dragged *dragged;
  filerwin     *fw;

  NOT_USED(event_no);

  dragged = &block->dragged;
  fw      = handle;

  if (fw->drag_type == drag_type_SELECTION_SELECT ||
      fw->drag_type == drag_type_SELECTION_ADJUST)
  {
    wimp_version_no    version;
    mapfn             *selector;
    wimp_window_state  wstate;

    xwimpreadsysinfo_version(&version);

    if (version >= wimp_VERSION_RO40)
      wimp_auto_scroll(0, NULL);

    // convert dragged.final to workarea relative
    // (note fiddling with dragged->final in place)

    wstate.w = fw->w;
    wimp_get_window_state(&wstate);

    screen_box_to_workarea(&dragged->final, &wstate);

    // for each icon: do we hit it?

    if (fw->drag_type == drag_type_SELECTION_SELECT)
    {
      bitvec_clear_all(fw->selection);
      selector = select_bobs;
    }
    else
    {
      selector = adjust_bobs;
    }

    map(fw, selector, 0,0, &dragged->final, NULL);


    fw->drag_type = -1;
  }

  return event_HANDLED;
}

static int filerwin_event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_key *key;
  filerwin *fw;

  NOT_USED(event_no);

  key = &block->key;
  fw  = handle;

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

filerwin *filerwin_create(void)
{
  char        *title_text;
  wimp_window  def;
  wimp_w       w;
  filerwin    *fw;

  def = wdef;

  title_text = malloc(256);
  if (title_text == NULL)
    return NULL;

  def.title_data.indirected_text.text = title_text;
  def.title_data.indirected_text.size = 256;

  w = wimp_create_window(&def);

  fw = malloc(sizeof(*fw));
  if (fw == NULL)
    return NULL;

  fw->w             = w;
  fw->redraw        = NULL;
  fw->pointer       = NULL;
  fw->title_text    = title_text;

  fw->object_width  = 80;
  fw->object_height = 120;
  fw->hpad          = 20;
  fw->vpad          = 60;
  fw->nobjects      = 0;
  fw->mode          = filerwin_mode_LARGE_ICONS;
  fw->sort          = filerwin_sort_NAME;

  fw->last_width    = 0;
  fw->last_height   = 0;
  fw->columns       = 0;
  fw->rows          = 0;

  fw->drag_type     = -1;

  fw->selection     = bitvec_create(0);
  if (fw->selection == NULL)
  {
    // deal
  }

  filerwin_internal_set_handlers(1, fw);

  return fw;
}

void filerwin_destroy(filerwin *doomed)
{
  if (doomed == NULL)
    return;

  filerwin_internal_set_handlers(0, doomed);

  bitvec_destroy(doomed->selection);

  free(doomed->title_text);

  wimp_delete_window(doomed->w);

  free(doomed);
}

wimp_w filerwin_get_window_handle(filerwin *fw)
{
  return fw->w;
}

void filerwin_set_handlers(filerwin            *fw,
                            filerwin_redrawfn  *redraw,
                            filerwin_closefn   *close,
                            filerwin_pointerfn *pointer)
{
  fw->redraw  = redraw;
  fw->close   = close;
  fw->pointer = pointer;
}

void filerwin_set_arg(filerwin *fw, void *arg)
{
  fw->arg = arg;
}

void filerwin_set_padding(filerwin *fw, int hpad, int vpad)
{
  fw->hpad = hpad;
  fw->vpad = vpad;
}

void filerwin_set_nobjects(filerwin *fw, int nobjects)
{
  fw->nobjects = nobjects;
}

void filerwin_set_dimensions(filerwin *fw, int width, int height)
{
  fw->object_width  = width;
  fw->object_height = height;
}

void filerwin_set_mode(filerwin *fw, filerwin_mode mode)
{
  fw->mode = mode;
}

void filerwin_set_sort(filerwin *fw, filerwin_sort sort)
{
  fw->sort = sort;
}

void filerwin_set_window_title(filerwin *fw, const char *title)
{
  window_set_title_text(fw->w, title);
}

void filerwin_select(filerwin *fw, int i)
{
  if (i < 0)
  {
    /* set the highest index, this forces the bitvec to its correct size.
     * this is needed as bitvec_set_all will only set up to the highest bit
     * index it's seen. */

    bitvec_set(fw->selection, fw->nobjects - 1);
    bitvec_set_all(fw->selection);
  }
  else
  {
    bitvec_set(fw->selection, i);
  }

  // selection changed partial redraw here
}

void filerwin_deselect(filerwin *fw, int i)
{
  if (i < 0)
    bitvec_clear_all(fw->selection);
  else
    bitvec_clear(fw->selection, i);

  // selection changed partial redraw here
}

void filerwin_open(filerwin *fw)
{
  wimp_window_state state;

  window_open_at(fw->w, AT_STAGGER);

  state.w = fw->w;
  wimp_get_window_state(&state);

  layout(fw, state.visible.x1 - state.visible.x0,
             state.visible.y1 - state.visible.y0);
}
