/* $Id: free.c,v 1.2 2008-08-05 22:04:51 dpt Exp $ */

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

void hlist_free(T *list)
{
  T next;

  assert(list != NULL);

  for ( ; *list; *list = next)
  {
    next = (*list)->rest;
    free(*list);
  }
}
