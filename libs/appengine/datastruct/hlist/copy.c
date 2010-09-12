/* $Id: copy.c,v 1.2 2008-08-05 22:04:51 dpt Exp $ */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

T hlist_copy(T list)
{
  T  head;
  T *p;

  p = &head;

  for ( ; list; list = list->rest)
  {
    *p = malloc(sizeof(**p));
    if (*p == NULL)
      return NULL; // doesn't unwind in failure case

    (*p)->first = list->first;
    p = &(*p)->rest;
  }

  *p = NULL;

  return head;
}
