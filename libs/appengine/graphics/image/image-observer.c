/* --------------------------------------------------------------------------
 *    Name: image-observer.c
 * Purpose: Informs clients when Image objects change
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/graphics/image.h"

#include "appengine/graphics/image-observer.h"

/* ----------------------------------------------------------------------- */

// FIXME: Used linked list library?
/* (image,callback,opaque) pairs are unique to an element. */
typedef struct element
{
  struct element         *next;
  image_t                *image; /* NULL => interested in all images */
  imageobserver_callback *callback;
  void                   *opaque;
  int                     nrefs;
}
element;

/* ----------------------------------------------------------------------- */

/* There's just one list of observers which is scanned when an event
 * arrives. */
static element *first = NULL;

/* ----------------------------------------------------------------------- */

int imageobserver_register(image_t                *image,
                           imageobserver_callback *callback,
                           void                   *opaque)
{
  element *e;

  /* Find existing matching element, if any */

  for (e = first; e != NULL; e = e->next)
    if (e->image == image && e->callback == callback && e->opaque == opaque)
      break;

  if (e) /* exists */
  {
    e->nrefs++;
    return 0; /* ok */
  }

  e = malloc(sizeof(*e));
  if (e == NULL)
    return 1; /* oom */

  e->image    = image;
  e->callback = callback;
  e->opaque   = opaque;
  e->nrefs    = 1;

  /* Insert at the start of the list */

  e->next     = first;
  first       = e;

  return 0; /* ok */
}

int imageobserver_deregister(image_t                *image,
                             imageobserver_callback *callback,
                             void                   *opaque)
{
  element *prev;
  element *e;
  element *next;

  /* Find existing matching element, if any */

  prev = NULL;
  next = NULL; /* shuts the compiler up - have I missed something here? */
  for (e = first; e != NULL; e = next)
  {
    next = e->next;

    if (e->image == image && e->callback == callback && e->opaque == opaque)
      break;

    prev = e;
  }

  /* {prev} -> {e} -> {next} */

  if (e == NULL)
    return 0; /* ok */

  if (--e->nrefs)
    return 0; /* there are more references remaining */

  if (prev == NULL)
    first = next;
  else
    prev->next = next;

  free(e);

  return 0; /* ok */
}

int imageobserver_register_greedy(imageobserver_callback *callback,
                                  void                   *opaque)
{
  element *e;

  /* Find existing matching element, if any */

  for (e = first; e != NULL; e = e->next)
    if (e->callback == callback && e->opaque == opaque)
      break;

  if (e) /* exists */
  {
    e->nrefs++;
    return 0; /* ok */
  }

  e = malloc(sizeof(*e));
  if (e == NULL)
    return 1; /* oom */

  e->image    = NULL;
  e->callback = callback;
  e->opaque   = opaque;
  e->nrefs    = 1;

  /* Insert at the start of the list */

  e->next     = first;
  first       = e;

  return 0; /* ok */
}

int imageobserver_deregister_greedy(imageobserver_callback *callback,
                                    void                   *opaque)
{
  element *prev;
  element *e;
  element *next;

  /* Find existing matching element, if any */

  prev = NULL;
  next = NULL; /* shuts the compiler up - have I missed something here? */
  for (e = first; e != NULL; e = next)
  {
    next = e->next;

    if (e->callback == callback && e->opaque == opaque)
      break;

    prev = e;
  }

  /* {prev} -> {e} -> {next} */

  if (e == NULL)
    return 0; /* ok */

  if (--e->nrefs)
    return 0; /* there are more references remaining */

  if (prev == NULL)
    first = next;
  else
    prev->next = next;

  free(e);

  return 0; /* ok */
}

int imageobserver_event(image_t              *image,
                        imageobserver_change  change,
                        imageobserver_data   *data)
{
  element *e;
  element *next;

  /* Event handlers can deregister whilst the list is being walked, so be
   * careful to always take the next pointer. Of course, if they happen to
   * delete any 'future' handlers too then we'll explode.
   */

  for (e = first; e != NULL; e = next)
  {
    next = e->next;

    if (e->image == NULL || e->image == image)
      e->callback(image, change, data, e->opaque);
  }

  return 0; /* ok */
}
