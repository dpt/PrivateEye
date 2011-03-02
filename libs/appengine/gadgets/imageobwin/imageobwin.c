/* --------------------------------------------------------------------------
 *    Name: imageobwin.c
 * Purpose: Image observer window
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/hourglass.h"
#include "oslib/osbyte.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/list.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/imageobwin.h"

/* ----------------------------------------------------------------------- */

static event_wimp_handler imageobwin__event_close_window_request;

static void imageobwin__close(imageobwin_t *obwin);

/* ----------------------------------------------------------------------- */

static void imageobwin__set_handlers(imageobwin_factory_t *factory,
                                     int reg, wimp_w w, void *handle)
{
  event_wimp_handler_spec wimp_handlers[4];

  wimp_handlers[0].event_no = wimp_REDRAW_WINDOW_REQUEST;
  wimp_handlers[1].event_no = wimp_CLOSE_WINDOW_REQUEST;
  wimp_handlers[2].event_no = wimp_MOUSE_CLICK;
  wimp_handlers[3].event_no = wimp_MENU_SELECTION;

  wimp_handlers[0].handler = factory->event_redraw_window_request;
  wimp_handlers[1].handler = imageobwin__event_close_window_request;
  wimp_handlers[2].handler = factory->event_mouse_click_request;
  wimp_handlers[3].handler = factory->event_menu_selection;

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            w, event_ANY_ICON,
                            handle);
}

/* ----------------------------------------------------------------------- */

static imageobwin_t *imageobwin__image_to_obwin(imageobwin_factory_t *factory,
                                                image_t              *image)
{
  return (imageobwin_t *) list__find(&factory->list_anchor,
                                     offsetof(imageobwin_t, image),
                               (int) image);
}

/* ----------------------------------------------------------------------- */

static void imageobwin__image_changed_callback(image_t              *image,
                                               imageobserver_change  change,
                                               imageobserver_data   *data,
                                               void                 *opaque)
{
  imageobwin_t *obwin = opaque;

  NOT_USED(data);

  switch (change)
  {
  case imageobserver_CHANGE_MODIFIED:
    /* Modifications to images can cause availability to change.
     * e.g. converting a JPEG to a sprite may discard metadata. */
    if (!obwin->factory->available(image))
      imageobwin__close(obwin);
    else
      imageobwin__kick(obwin);
    break;

  case imageobserver_CHANGE_HIDDEN:
  case imageobserver_CHANGE_ABOUT_TO_DESTROY:
    imageobwin__close(obwin);
    break;
  }
}

/* ----------------------------------------------------------------------- */

error imageobwin__construct(imageobwin_factory_t *factory,
                            const char           *name,
                            window_open_at_flags  open_at,
                            imageobwin_available *available,
                            imageobwin_alloc     *alloc,
                            imageobwin_dealloc   *dealloc,
                            imageobwin_compute   *compute,
                            imageobwin_refresh   *refresh,
                            event_wimp_handler   *redraw,
                            event_wimp_handler   *click,
                            event_wimp_handler   *menu)
{
    error err;
    char  scratch[32]; /* Careful Now */

    err = help__init();
    if (err)
        return err;

    /* init */

    factory->w = window_create(name);

    sprintf(scratch, "menu.%s", name);
    factory->menu = menu_create_from_desc(message0(scratch));
    if (factory->menu == NULL)
    {
        help__fin();
        return error_OOM;
    }

    err = help__add_menu(factory->menu, name);
    if (err)
    {
        menu_destroy(factory->menu);
        help__fin();
        return err;
    }

    list__init(&factory->list_anchor);

    factory->name = malloc(strlen(name) + 1);
    if (factory->name == NULL)
    {
        help__remove_menu(factory->menu);
        menu_destroy(factory->menu);
        help__fin();
        return error_OOM;
    }
    strcpy(factory->name, name);

    factory->open_at                     = open_at;

    factory->available                   = available;
    factory->alloc                       = alloc;
    factory->dealloc                     = dealloc;
    factory->compute                     = compute;
    factory->refresh                     = refresh;

    factory->event_redraw_window_request = redraw;
    factory->event_mouse_click_request   = click;
    factory->event_menu_selection        = menu;

    return error_OK;
}

void imageobwin__destruct(imageobwin_factory_t *doomed)
{
    free(doomed->name);

    help__remove_menu(doomed->menu);
    menu_destroy(doomed->menu);
    help__fin();
}

static int imageobwin__event_close_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  imageobwin_t *obwin = handle;

  NOT_USED(event_no);
  NOT_USED(block);

  imageobwin__close(obwin);

  return event_HANDLED;
}

static error imageobwin__new(imageobwin_factory_t *factory,
                             image_t              *image,
                             const void           *config,
                             imageobwin_t        **new_obwin)
{
  error         err;
  imageobwin_t *obwin;
  char          scratch[32];
  const char   *leaf;
  char          title[256];

  *new_obwin = NULL;

  /* call the client to allocate a new imageobwin */

  err = factory->alloc(config, &obwin);
  if (err)
      return err;

  /* clone ourselves a window */

  obwin->w = window_clone(factory->w);
  if (obwin->w == NULL)
    goto NoMem;

  /* set its title, including the leafname of the image */

  sprintf(scratch, "%s.title", factory->name);
  leaf = str_leaf(image->file_name);
  sprintf(title, message0(scratch), leaf);
  window_set_title_text(obwin->w, title);

  /* fill out */

  obwin->image   = image;
  obwin->factory = factory;

  imageobwin__set_handlers(factory, 1, obwin->w, obwin);

  err = help__add_window(obwin->w, factory->name);
  if (err)
    return err;

  /* watch for changes */

  imageobserver_register(image, imageobwin__image_changed_callback, obwin);

  /* link it in */

  list__add_to_head(&factory->list_anchor, &obwin->list);

  *new_obwin = obwin;

  return error_OK;


NoMem:

  if (obwin)
  {
    window_delete_cloned(obwin->w);

    factory->dealloc(obwin);
  }

  return error_OOM;
}

error imageobwin__open(imageobwin_factory_t *factory,
                       image_t              *image,
                       const void           *config)
{
  error         err;
  imageobwin_t *obwin;

  if (!factory->available(image))
  {
    beep();
    return error_OK;
  }

  obwin = imageobwin__image_to_obwin(factory, image);
  if (obwin == NULL)
  {
    err = imageobwin__new(factory, image, config, &obwin);
    if (!err)
      err = factory->compute(obwin);
    if (err)
      return err;
  }

  window_open_at(obwin->w, factory->open_at);

  return error_OK;
}

static void imageobwin__close(imageobwin_t *obwin)
{
  imageobserver_deregister(obwin->image, imageobwin__image_changed_callback, obwin);

  help__remove_window(obwin->w);

  imageobwin__set_handlers(obwin->factory, 0, obwin->w, obwin);

  list__remove(&obwin->factory->list_anchor, &obwin->list);

  window_delete_cloned(obwin->w);

  obwin->factory->dealloc(obwin); /* client callback, destroys structure so
                                     must come last */
}

void imageobwin__kick(imageobwin_t *obwin)
{
  error err;

  if (obwin == NULL)
    return;

  if (!obwin->factory->available(obwin->image))
    return;

  err = obwin->factory->compute(obwin);
  if (err)
  {
    error__report(err);
    return;
  }

  obwin->factory->refresh(obwin);
}
