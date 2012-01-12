/* --------------------------------------------------------------------------
 *    Name: list.h
 * Purpose: Linked list library
 * ----------------------------------------------------------------------- */

/**
 * \file Linked list (interface).
 *
 * List is a linked list.
 */

#ifndef APPENGINE_LIST_H
#define APPENGINE_LIST_H

#include <stdlib.h>

#define T list_t

typedef struct T
{
  struct T *next;
}
T;

void list_init(T *anchor);

/* Anchor is assumed to be a static element whose only job is to point to
 * the first element in the list. */

void list_add_to_head(T *anchor, T *item);

void list_remove(T *anchor, T *doomed);

typedef int (list_walk_callback)(T *, void *);

void list_walk(T *anchor, list_walk_callback *cb, void *opaque);

/* Searches the linked list looking for a key. The key is specified as an
 * offset from the start of the linked list element. It is an int-sized unit.
 */
T *list_find(T *anchor, size_t keyloc, int key);

#undef T

#endif /* APPENGINE_LIST_H */
