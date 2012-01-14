/* --------------------------------------------------------------------------
 *    Name: imageobwin.h
 * Purpose: Image observer window
 * ----------------------------------------------------------------------- */

/* Two base classes which implement the behaviour required by image observing
 * windows including the histogram and metadata windows.
 *
 * As with the dialogue code this calls no memory allocation functions itself
 * so calls back into the client to get stuff allocated or deallocated.
 * Structures are exposed to allow them to be embedded as the first member of
 * client structures. */

#ifndef APPENGINE_IMAGEOBWIN_H
#define APPENGINE_IMAGEOBWIN_H

#include "appengine/base/errors.h"
#include "appengine/datastruct/list.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/window.h"

/* Factory class - manufactures imageobwin_t's. */
typedef struct imageobwin_factory_t imageobwin_factory_t;
/* Image observer window class. */
typedef struct imageobwin_t imageobwin_t;

/* Asks if the specified image is suitable for use by the derived class. */
typedef int (imageobwin_available)(const image_t *image);

/* Asks for a new part-configured derived class of imageobwin_t. */
typedef error (imageobwin_alloc)(const void    *config,
                                 imageobwin_t **obwin);

/* Asks for the derived class of imageobwin_t to be deallocated. */
typedef void (imageobwin_dealloc)(imageobwin_t *doomed);

/* Asks the derived class to recompute its data when the observed image
 * changes. */
typedef error (imageobwin_compute)(imageobwin_t *obwin);

/* Asks the derived class to refresh its display. */
typedef void (imageobwin_refresh)(imageobwin_t *obwin);

struct imageobwin_factory_t
{
  wimp_w                w;           /* window from which we clone each new
                                        instance's window */
  wimp_menu            *menu;        /* used for _every_ window (itself a
                                        singleton) (or NULL) */
  list_t                list_anchor; /* linked list of observer windows of
                                        this class */

  char                 *name;

  window_open_at_flags  open_at;

  imageobwin_available *available;
  imageobwin_alloc     *alloc;
  imageobwin_dealloc   *dealloc;
  imageobwin_compute   *compute;
  imageobwin_refresh   *refresh;

  event_wimp_handler   *event_redraw_window_request;
  event_wimp_handler   *event_mouse_click_request;
  event_wimp_handler   *event_menu_selection;
};

struct imageobwin_t
{
  list_t                list;    /* each is a linked list node */
  wimp_w                w;       /* our window */
  image_t              *image;   /* image we're observing */
  imageobwin_factory_t *factory; /* who manufactured us */
};

/* Constructs a new imageobwin_factory_t. */
error imageobwin_construct(imageobwin_factory_t  *self,
                           const char            *name,
                           window_open_at_flags   open_at,
                           imageobwin_available  *available,
                           imageobwin_alloc      *alloc,
                           imageobwin_dealloc    *dealloc,
                           imageobwin_compute    *compute,
                           imageobwin_refresh    *refresh,
                           event_wimp_handler    *redraw,
                           event_wimp_handler    *click,
                           event_wimp_handler    *menu);

void imageobwin_destruct(imageobwin_factory_t *doomed);

/* Called to create and open a new image observer window. */
error imageobwin_open(imageobwin_factory_t *factory,
                      image_t              *image,
                      const void           *config);

/* Compute then refresh the window. */
void imageobwin_kick(imageobwin_t *obwin);

#endif /* APPENGINE_IMAGEOBWIN_H */

