/* $Id: find.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include "appengine/datastruct/list.h"

list_t *list__find(list_t *anchor,
                   size_t  keyloc,
                   int     key)
{
  list_t *e;

#define GET_KEY(element, offset) (*((int *) ((char *) element + offset)))

  for (e = anchor; e != NULL; e = e->next)
    if (GET_KEY(e, keyloc) == key)
      return e;

  return NULL;
}
