/* $Id: reverse.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

T hlist_reverse(T list)
{
  T head;
  T next;

  head = NULL;

  for ( ; list; list = next)
  {
    next = list->rest;
    list->rest = head;
    head = list;
  }

  return head;
}
