/* --------------------------------------------------------------------------
 *    Name: hlist.h
 * Purpose: "Hanson" linked lists
 * Version: $Id: hlist.h,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

/* See chapter 5 of C Interfaces and Implementations. */

#ifndef APPENGINE_HLIST_H
#define APPENGINE_HLIST_H

#define T hlist_t

typedef struct T *T;

struct T
{
  T     rest;
  void *first; /* the payload */
};

T hlist_append(T list, T tail);
T hlist_copy(T list);
T hlist_list(void *x, ...);
T hlist_pop(T list, void **x);
T hlist_push(T list, void *x);
T hlist_reverse(T list);
int hlist_length(T list);
void hlist_free(T *list);
void hlist_map(T list, void apply(void **x, void *cl),  void *cl);
void **hlist_to_array(T list, void *end);

#undef T

#endif /* APPENGINE_HLIST_H */
