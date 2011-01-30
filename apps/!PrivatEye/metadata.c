/* --------------------------------------------------------------------------
 *    Name: metadata.c
 * Purpose: Metadata
 * Version: $Id: metadata.c,v 1.16 2009-11-29 23:18:36 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifdef EYE_META

/* TODO
 *
 * There's an awful lot of common ground between this module and the
 * histogram window. Perhaps I should factor out the common code into an
 * image observing window base class.
 */

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/hourglass.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/datastruct/list.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/gadgets/treeview.h"
#include "appengine/wimp/window.h"

#include "globals.h"
#include "menunames.h"

#include "metadata.h"

typedef struct metadata_window
{
  list_t      list;
  image_t    *image;
  wimp_w      w;
  treeview_t *tr;
}
metadata_window;

static list_t list_anchor;

static struct
{
  wimp_w           w;
  wimp_menu       *menu;
  metadata_window *last_md;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static event_wimp_handler metadata__event_redraw_window_request,
                          metadata__event_close_window_request,
                          metadata__event_mouse_click,
                          metadata__event_menu_selection;

/* ----------------------------------------------------------------------- */

static void metadata__set_handlers(int reg, wimp_w w, void *handle)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, metadata__event_redraw_window_request },
    { wimp_CLOSE_WINDOW_REQUEST,  metadata__event_close_window_request  },
    { wimp_MOUSE_CLICK,           metadata__event_mouse_click           },
    { wimp_MENU_SELECTION,        metadata__event_menu_selection        },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            w, event_ANY_ICON,
                            handle);
}

/* ----------------------------------------------------------------------- */

static int metadata__refcount = 0;

error metadata__init(void)
{
  error err;

  if (metadata__refcount++ == 0)
  {
    /* dependencies */

    err = help__init();
    if (err)
      return err;

    err = treeview__init();
    if (err)
      goto Failure;

    /* handlers */

    LOCALS.w = window_create("metadata");

    LOCALS.menu = menu_create_from_desc(message0("menu.metadata"));

    err = help__add_menu(LOCALS.menu, "metadata");
    if (err)
      return err;

    list__init(&list_anchor);
  }

  err = error_OK;

  /* FALLTHROUGH */

Failure:

  return err;
}

void metadata__fin(void)
{
  if (--metadata__refcount == 0)
  {
    help__remove_menu(LOCALS.menu);

    menu_destroy(LOCALS.menu);

    treeview__fin();

    help__fin();
  }
}

/* ----------------------------------------------------------------------- */

static metadata_window *metadata__image_to_win(image_t *image)
{
  return (metadata_window *) list__find(&list_anchor,
                                         offsetof(metadata_window, image),
                                   (int) image);
}

/* ----------------------------------------------------------------------- */

static void metadata__delete(metadata_window *md);

/* ----------------------------------------------------------------------- */

static error metadata__compute(image_t *image)
{
  error              err;
  metadata_window   *md;
  ntree_t           *tree;
  int                w,h;
  os_box             box;

  if ((image->flags & image_FLAG_HAS_META) == 0)
    return error_PRIVATEEYE_META_UNSUPP_FUNC;

  md = metadata__image_to_win(image); /* FIXME: Wasteful repetition of
                                         operation already done in the outer
                                         scope. */

  hourglass_on();

  if (image->methods.get_meta(&GLOBALS.choices.image, image, &tree))
  {
    /* FIXME: Cleanup. */
    return error_PRIVATEEYE_META_UNSUPP_FUNC;
  }

  hourglass_off();

  err = treeview__create(&md->tr);
  if (err)
    goto Failure;

  treeview__set_text_width(md->tr, GLOBALS.choices.metadata.wrapwidth);

  treeview__set_line_height(md->tr, GLOBALS.choices.metadata.line_height);

  treeview__set_highlight_background(md->tr, GLOBALS.choices.metadata.bgcolour);

  treeview__set_tree(md->tr, tree);

  image_destroy_metadata(tree);

  treeview__make_collapsible(md->tr);

  err = treeview__get_dimensions(md->tr, &w, &h);
  if (err)
    goto Failure;

  box.x0 = 0;
  box.y0 = h; /* this is negative */
  box.x1 = w;
  box.y1 = 0;
  wimp_set_extent(md->w, &box);

  return error_OK;


Failure:

  return err;
}

/* ----------------------------------------------------------------------- */

static int metadata__event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_draw        *draw;
  metadata_window  *md;
  int               more;

  NOT_USED(event_no);
  NOT_USED(handle);

  draw = &block->redraw;
  md   = handle;

  for (more = wimp_redraw_window(draw); more; more = wimp_get_rectangle(draw))
  {
    treeview__draw(md->tr);
  }

  return event_HANDLED;
}

static int metadata__event_close_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_close      *close;
  metadata_window *md;

  NOT_USED(event_no);
  NOT_USED(handle);

  close = &block->close;

  md = handle;

  metadata__delete(md);

  return event_HANDLED;
}

static void metadata__refresh(metadata_window *md)
{
  error err;

  err = metadata__compute(md->image);
}

static int metadata__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer    *pointer;
  metadata_window *md;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  md = handle;

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

        (void) treeview__click(md->tr, x, y, &redraw_y);

        if (redraw_y <= 0)
          wimp_force_redraw(pointer->w, 0, -32768, 32767, redraw_y);
      }
      break;
    }
  }
  else if (pointer->buttons & wimp_CLICK_MENU)
  {
    LOCALS.last_md = md;
    menu_open(LOCALS.menu, pointer->pos.x - 64, pointer->pos.y);
  }

  return event_HANDLED;
}

static int metadata__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_selection  *selection;
  wimp_menu       *last;
  metadata_window *md;
  treeview__mark   mark;
  wimp_pointer     p;

  NOT_USED(event_no);
  NOT_USED(handle);

  selection = &block->selection;

  last = menu_last();
  if (last != LOCALS.menu)
    return event_NOT_HANDLED;

  md = LOCALS.last_md;

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

  treeview__mark_all(md->tr, mark);

  wimp_force_redraw(md->w, 0, -32768, 32767, 0);

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
    menu_reopen();

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

int metadata__available(const image_t *image)
{
  return image &&
         image->flags & image_FLAG_HAS_META;
}

static void metadata__image_changed_callback(image_t              *image,
                                             imageobserver_change  change,
                                             imageobserver_data   *data)
{
  metadata_window *md;

  NOT_USED(data);

  md = metadata__image_to_win(image);

  switch (change)
  {
  case imageobserver_CHANGE_MODIFIED:
    metadata__refresh(md);
    break;

  case imageobserver_CHANGE_HIDDEN:
  case imageobserver_CHANGE_ABOUT_TO_DESTROY:
    imageobserver_deregister(image, metadata__image_changed_callback);
    metadata__delete(md);
    break;
  }
}

static error metadata__new(image_t *image, metadata_window **new_md)
{
  error            err;
  metadata_window *md;
  const char      *leaf;
  char             title[256];

  /* no window for this image */

  md = malloc(sizeof(*md));
  if (md == NULL)
    goto NoMem;

  /* clone ourselves a window */

  md->w = window_clone(LOCALS.w);
  if (md->w == NULL)
    goto NoMem;

  /* set its title, including the leafname of the image */

  leaf = str_leaf(image->file_name);
  sprintf(title, message0("metadata.title"), leaf);
  window_set_title_text(md->w, title);

  /* fill out */

  md->image        = image;
  md->tr           = NULL;

  list__add_to_head(&list_anchor, &md->list);

  metadata__set_handlers(1, md->w, md);

  err = help__add_window(md->w, "metadata");
  if (err)
    return err;

  /* watch for changes */

  imageobserver_register(image, metadata__image_changed_callback);

  *new_md = md;

  return error_OK;


NoMem:

  if (md)
    window_delete_cloned(md->w);

  free(md);

  return error_OOM;
}

static void metadata__delete(metadata_window *md)
{
  imageobserver_deregister(md->image, metadata__image_changed_callback);

  help__remove_window(md->w);

  metadata__set_handlers(0, md->w, md);

  list__remove(&list_anchor, &md->list);

  treeview__destroy(md->tr);

  window_delete_cloned(md->w);

  free(md);
}

/* ----------------------------------------------------------------------- */

void metadata__open(image_t *image)
{
  error            err;
  metadata_window *md;

  if (!metadata__available(image))
  {
    beep();
    return;
  }

  md = metadata__image_to_win(image);
  if (md == NULL)
  {
    err = metadata__new(image, &md);
    if (err)
      goto Failure;

    err = metadata__compute(image);
    if (err)
      goto Failure;
  }

  window_open_at(md->w, AT_DEFAULT);

  return;


Failure:

  error__report(err);

  return;
}

#else

extern int dummy;

#endif /* EYE_META */
