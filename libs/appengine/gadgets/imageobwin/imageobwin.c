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

#include "datastruct/list.h"

#include "appengine/types.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/strings.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/imageobwin.h"

/* ----------------------------------------------------------------------- */

static event_wimp_handler imageobwin_event_close_window_request;

static void imageobwin_close(imageobwin_t *obwin);

/* ----------------------------------------------------------------------- */

static void imageobwin_set_handlers(imageobwin_factory_t *factory,
                                    int                   reg,
                                    wimp_w                w,
                                    void                 *handle)
{
  event_wimp_handler_spec wimp_handlers[4];
  int i;

  i = 0;
  wimp_handlers[i].event_no = wimp_REDRAW_WINDOW_REQUEST;
  wimp_handlers[i].handler = factory->event_redraw_window_request;
  i++;
  
  wimp_handlers[i].event_no = wimp_CLOSE_WINDOW_REQUEST;
  wimp_handlers[i].handler = imageobwin_event_close_window_request;
  i++;
  
  wimp_handlers[i].event_no = wimp_MOUSE_CLICK;
  wimp_handlers[i].handler = factory->event_mouse_click_request;
  i++;

  if (factory->event_menu_selection)
  {
    wimp_handlers[i].event_no = wimp_MENU_SELECTION;
    wimp_handlers[i].handler = factory->event_menu_selection;
    i++;
  }

  event_register_wimp_group(reg,
                            wimp_handlers,
                            i,
                            w,
                            event_ANY_ICON,
                            handle);
}

/* ----------------------------------------------------------------------- */

static imageobwin_t *imageobwin_image_to_obwin(imageobwin_factory_t *factory,
                                               image_t              *image)
{
  return (imageobwin_t *) list_find(&factory->list_anchor,
                                     offsetof(imageobwin_t, image),
                               (int) image);
}

/* ----------------------------------------------------------------------- */

static void imageobwin_image_changed_callback(image_t              *image,
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
      imageobwin_close(obwin);
    else
      imageobwin_kick(obwin);
    break;

  case imageobserver_CHANGE_HIDDEN:
  case imageobserver_CHANGE_ABOUT_TO_DESTROY:
    imageobwin_close(obwin);
    break;
  }
}

/* ----------------------------------------------------------------------- */

result_t imageobwin_construct(imageobwin_factory_t *factory,
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
  result_t err;
  char     token[32]; /* Careful Now */
  size_t   len;

  /* initialise member(s) used in error cleanup */

  factory->menu = NULL;

  /* dependencies */

  err = help_init();
  if (err)
    return err;

  /* init */

  factory->w = window_create(name);

  if (menu)
  {
    sprintf(token, "menu.%s", name);
    factory->menu = menu_create_from_desc(message0(token));
    if (factory->menu == NULL)
    {
      err = result_OOM;
      goto Failure;
    }

    err = help_add_menu(factory->menu, name);
    if (err)
      goto Failure;
  }

  list_init(&factory->list_anchor);

  len = strlen(name) + 1;
  factory->name = malloc(len);
  if (factory->name == NULL)
  {
    if (menu)
      help_remove_menu(factory->menu);
    goto Failure;
  }
  memcpy(factory->name, name, len);

  factory->open_at                     = open_at;

  factory->available                   = available;
  factory->alloc                       = alloc;
  factory->dealloc                     = dealloc;
  factory->compute                     = compute;
  factory->refresh                     = refresh;

  factory->event_redraw_window_request = redraw;
  factory->event_mouse_click_request   = click;
  factory->event_menu_selection        = menu;

  return result_OK;

Failure:

  if (menu)
    menu_destroy(factory->menu);

  help_fin();

  return err;
}

void imageobwin_destruct(imageobwin_factory_t *doomed)
{
  if (doomed == NULL)
    return;

  free(doomed->name);

  if (doomed->menu)
  {
    help_remove_menu(doomed->menu);
    menu_destroy(doomed->menu);
  }

  help_fin();
}

static int imageobwin_event_close_window_request(wimp_event_no  event_no,
                                                 wimp_block    *block,
                                                 void          *handle)
{
  imageobwin_t *obwin = handle;

  NOT_USED(event_no);
  NOT_USED(block);

  imageobwin_close(obwin);

  return event_HANDLED;
}

static result_t imageobwin_new(imageobwin_factory_t *factory,
                               image_t              *image,
                               const void           *config,
                               imageobwin_t        **new_obwin)
{
  result_t      err;
  imageobwin_t *obwin;
  char          token[32];
  char          format[32];
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
  {
    err = result_OOM;
    goto Failure;
  }

  err = help_add_window(obwin->w, factory->name);
  if (err)
    goto Failure;

  /* set its title, including the leafname of the image */

  sprintf(token, "%s.title", factory->name);
  strcpy(format, message0(token));
  if (image->file_name[0] != '\0')
    leaf = str_leaf(image->file_name);
  else
    leaf = message0("untitled.title");
  sprintf(title, format, leaf);
  window_set_title_text(obwin->w, title);

  /* fill out */

  obwin->image   = image;
  obwin->factory = factory;

  imageobwin_set_handlers(factory, 1, obwin->w, obwin);

  /* watch for changes */

  imageobserver_register(image, imageobwin_image_changed_callback, obwin);

  /* link it in */

  list_add_to_head(&factory->list_anchor, &obwin->list);

  *new_obwin = obwin;

  return result_OK;

Failure:

  if (obwin)
  {
    window_delete_cloned(obwin->w);

    factory->dealloc(obwin);
  }

  return result_OOM;
}

result_t imageobwin_open(imageobwin_factory_t *factory,
                         image_t              *image,
                         const void           *config)
{
  result_t      err;
  imageobwin_t *obwin;

  if (!factory->available(image))
  {
    beep();
    return result_OK;
  }

  obwin = imageobwin_image_to_obwin(factory, image);
  if (obwin == NULL)
  {
    err = imageobwin_new(factory, image, config, &obwin);
    if (!err)
      err = factory->compute(obwin);
    if (err)
      return err;
  }

  window_open_at(obwin->w, factory->open_at);

  return result_OK;
}

static void imageobwin_close(imageobwin_t *obwin)
{
  imageobserver_deregister(obwin->image,
                           imageobwin_image_changed_callback,
                           obwin);

  help_remove_window(obwin->w);

  imageobwin_set_handlers(obwin->factory, 0, obwin->w, obwin);

  list_remove(&obwin->factory->list_anchor, &obwin->list);

  window_delete_cloned(obwin->w);

  obwin->factory->dealloc(obwin); /* client destroys structure so this must
                                     come last */
}

void imageobwin_kick(imageobwin_t *obwin)
{
  result_t err;

  if (obwin == NULL)
    return;

  if (!obwin->factory->available(obwin->image))
    return;

  err = obwin->factory->compute(obwin);
  if (err)
  {
    result_report(err);
    return;
  }

  obwin->factory->refresh(obwin);
}

