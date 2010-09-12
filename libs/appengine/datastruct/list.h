/* --------------------------------------------------------------------------
 *    Name: list.h
 * Purpose: Linked list abstraction
 * Version: $Id: list.h,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_LIST_H
#define APPENGINE_LIST_H

#include <stdlib.h>

#define T list_t

typedef struct T
{
  struct T *next;
}
T;

void list__init(T *anchor);

/* Anchor is assumed to be a static element whose only job is to point to
 * the first element in the list. */

void list__add_to_head(T *anchor, T *item);

void list__remove(T *anchor, T *doomed);

typedef int (list__walk_callback)(T *, void *);

void list__walk(T *anchor, list__walk_callback *cb, void *cbarg);

/* Searches the linked list looking for a key. The key is specified as an
 * offset from the start of the linked list element. It is an int-sized unit.
 */
T *list__find(T *anchor, size_t keyloc, int key);

#undef T

#endif /* APPENGINE_LIST_H */
