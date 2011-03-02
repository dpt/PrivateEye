/* --------------------------------------------------------------------------
 *    Name: metadata.c
 * Purpose: Metadata windows
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/hourglass.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/gadgets/imageobwin.h"
#include "appengine/gadgets/treeview.h"
#include "appengine/graphics/image.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/metadata.h"

/* ----------------------------------------------------------------------- */

static imageobwin_alloc   metadata__alloc;
static imageobwin_dealloc metadata__dealloc;
static imageobwin_compute metadata__compute;
static imageobwin_refresh metadata__refresh;

static event_wimp_handler metadata__event_redraw_window_request,
                          metadata__event_mouse_click,
                          metadata__event_menu_selection;

/* ----------------------------------------------------------------------- */

/* Menus */
enum
{
  METADATA_EXPAND_ALL,
  METADATA_COLLAPSE_ALL,
};

/* ----------------------------------------------------------------------- */

typedef struct metadata_config
{
  os_colour bgcolour;   /* colour to draw highlights */
  int       wrapwidth;  /* column width to wrap at */
  int       lineheight; /* OS unit height of each line */
}
metadata_config;

/* ----------------------------------------------------------------------- */

typedef struct metadata_window
{
  imageobwin_t     base; /* a metadata_window is an image observer window */

  metadata_config  config;

  treeview_t      *tr;   /* treeview data */
}
metadata_window;

static struct
{
  imageobwin_factory_t factory;
  unsigned int         refcount;
}
LOCALS;

/* ----------------------------------------------------------------------- */

error metadata__init(void)
{
  error err;

  if (LOCALS.refcount)
      return error_OK; /* already initialised */

  /* dependencies */

  err = treeview__init();
  if (err)
    return err;

  err = imageobwin__construct(&LOCALS.factory,
                              "metadata",
                              AT_DEFAULT,
                              metadata__available,
                              metadata__alloc,
                              metadata__dealloc,
                              metadata__compute,
                              metadata__refresh,
                              metadata__event_redraw_window_request,
                              metadata__event_mouse_click,
                              metadata__event_menu_selection);
  if (err)
  {
    treeview__fin();
    return err;
  }

  LOCALS.refcount++;

  return error_OK;
}

void metadata__fin(void)
{
  if (LOCALS.refcount == 0)
    return;

  imageobwin__destruct(&LOCALS.factory);

  treeview__fin();

  LOCALS.refcount--;
}

/* ----------------------------------------------------------------------- */

void metadata__open(image_t *image,
                    os_colour bgcolour, int wrapwidth, int lineheight)
{
  metadata_config config;

  config.bgcolour   = bgcolour;
  config.wrapwidth  = wrapwidth;
  config.lineheight = lineheight;

  imageobwin__open(&LOCALS.factory, image, &config);
}

int metadata__available(const image_t *image)
{
  return image &&
         image->flags & image_FLAG_HAS_META;
}

/* ----------------------------------------------------------------------- */

static error metadata__alloc(const void    *opaque_config,
                             imageobwin_t **obwin)
{
  metadata_window       *self;
  const metadata_config *config = opaque_config;

  *obwin = NULL;

  self = malloc(sizeof(*self));
  if (self == NULL)
      return error_OOM;

  self->config = *config;

  self->tr     = NULL;

  *obwin = &self->base;

  return error_OK;
}

static void metadata__dealloc(imageobwin_t *doomed)
{
  metadata_window *self = (metadata_window *) doomed;

  treeview__destroy(self->tr);

  free(self);
}

static error metadata__compute(imageobwin_t *obwin)
{
  error            err;
  metadata_window *self;
  image_t         *image;
  ntree_t         *tree;
  int              w,h;
  os_box           box;

  self  = (metadata_window *) obwin;
  image = self->base.image;

  hourglass_on();

  if (image->methods.get_meta(image, &tree))
    return error_PRIVATEEYE_META_UNSUPP_FUNC;

  hourglass_off();

  err = treeview__create(&self->tr);
  if (err)
    goto Failure;

  treeview__set_text_width(self->tr, self->config.wrapwidth);
  treeview__set_line_height(self->tr, self->config.lineheight);
  treeview__set_highlight_background(self->tr, self->config.bgcolour);

  treeview__set_tree(self->tr, tree);

  image_destroy_metadata(tree);

  treeview__make_collapsible(self->tr);

  err = treeview__get_dimensions(self->tr, &w, &h);
  if (err)
    goto Failure;

  box.x0 = 0;
  box.y0 = h; /* this is negative */
  box.x1 = w;
  box.y1 = 0;
  wimp_set_extent(self->base.w, &box);

  return error_OK;


Failure:

  return err;
}

static void metadata__refresh(imageobwin_t *obwin)
{
  metadata_window *self = (metadata_window *) obwin;

  wimp_force_redraw(self->base.w, 0, -32768, 32767, 0);
}

/* ----------------------------------------------------------------------- */

static int metadata__event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  metadata_window *self;
  wimp_draw       *draw;
  int              more;

  NOT_USED(event_no);

  self = handle;
  draw = &block->redraw;

  for (more = wimp_redraw_window(draw); more; more = wimp_get_rectangle(draw))
    treeview__draw(self->tr);

  return event_HANDLED;
}

static int metadata__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  metadata_window *self;
  wimp_pointer    *pointer;

  NOT_USED(event_no);

  self    = handle;
  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case wimp_ICON_WINDOW:
      {
        wimp_window_state state;
        int               x,y;
        int               redraw_y;

        /* convert pointer position to window relative */

        state.w = pointer->w;
        wimp_get_window_state(&state);

        x = pointer->pos.x + (state.xscroll - state.visible.x0);
        y = pointer->pos.y + (state.yscroll - state.visible.y1);

        (void) treeview__click(self->tr, x, y, &redraw_y);

        if (redraw_y <= 0)
          wimp_force_redraw(pointer->w, 0, -32768, 32767, redraw_y);
      }
      break;
    }
  }
  else if (pointer->buttons & wimp_CLICK_MENU)
  {
    menu_open(self->base.factory->menu, pointer->pos.x - 64, pointer->pos.y);
  }

  return event_HANDLED;
}

static int metadata__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
  metadata_window *self;
  wimp_selection  *selection;
  wimp_menu       *last;
  treeview__mark   mark;
  wimp_pointer     p;

  NOT_USED(event_no);

  self      = handle;
  selection = &block->selection;

  last = menu_last();
  if (last != self->base.factory->menu)
    return event_NOT_HANDLED;

  switch (selection->items[0])
  {
  default:
  case METADATA_EXPAND_ALL:
    mark = treeview__mark_EXPAND;
    break;
  case METADATA_COLLAPSE_ALL:
    mark = treeview__mark_COLLAPSE;
    break;
  }

  treeview__mark_all(self->tr, mark);

  wimp_force_redraw(self->base.w, 0, -32768, 32767, 0);

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
    menu_reopen();

  return event_HANDLED;
}

