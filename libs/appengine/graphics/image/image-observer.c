/* --------------------------------------------------------------------------
 *    Name: image-observer.c
 * Purpose: Informs clients when Image objects change
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/graphics/image.h"

#include "appengine/graphics/image-observer.h"

/* ----------------------------------------------------------------------- */

/* (image,callback,opaque) identifies a single observer */
typedef struct observer
{
  struct observer        *next;
  image_t                *image; /* NULL if interested in all images */
  imageobserver_callback *callback;
  void                   *opaque;
  int                     nrefs;
}
observer;

/* ----------------------------------------------------------------------- */

/* There's just one list of observers which is scanned when an event
 * arrives. */
static observer *first_observer = NULL;

/* ----------------------------------------------------------------------- */

int imageobserver_register(image_t                *image,
                           imageobserver_callback *callback,
                           void                   *opaque)
{
  observer *o;

  /* Find existing matching observer, if any */
  for (o = first_observer; o != NULL; o = o->next)
    if (o->image == image && o->callback == callback && o->opaque == opaque)
      break;

  if (o) /* exists */
  {
    o->nrefs++;
    return 0; /* ok */
  }

  o = malloc(sizeof(*o));
  if (o == NULL)
    return 1; /* oom */

  o->image    = image;
  o->callback = callback;
  o->opaque   = opaque;
  o->nrefs    = 1;

  /* Insert at the start of the list */
  o->next = first_observer;
  first_observer = o;

  return 0; /* ok */
}

int imageobserver_deregister(image_t                *image,
                             imageobserver_callback *callback,
                             void                   *opaque)
{
  observer *prev;
  observer *next;
  observer *o;

  /* Find existing matching observer, if any */
  prev = next = NULL;
  for (o = first_observer; o != NULL; o = next)
  {
    next = o->next;

    if (o->image == image && o->callback == callback && o->opaque == opaque)
      break;

    prev = o;
  }

  /* At this point: prev -> o -> next */

  if (o == NULL)
    return 0; /* not found (ok) */

  if (--o->nrefs)
    return 0; /* there are more references remaining */

  if (prev == NULL)
    first_observer = next;
  else
    prev->next = next;

  free(o);

  return 0; /* ok */
}

int imageobserver_register_greedy(imageobserver_callback *callback,
                                  void                   *opaque)
{
  return imageobserver_register(NULL, callback, opaque);
}

int imageobserver_deregister_greedy(imageobserver_callback *callback,
                                    void                   *opaque)
{
  return imageobserver_deregister(NULL, callback, opaque);
}

int imageobserver_event(image_t              *image,
                        imageobserver_change  change,
                        imageobserver_data   *data)
{
  observer *o;
  observer *next;

  /* Event handlers can deregister whilst the list is being walked, so be
   * careful to always take the next pointer. Of course, if they happen to
   * delete any 'future' handlers too then we'll explode.
   */

  for (o = first_observer; o != NULL; o = next)
  {
    next = o->next;

    if (o->image == NULL || o->image == image)
      o->callback(image, change, data, o->opaque);
  }

  return 0; /* ok */
}
