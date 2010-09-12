/* $Id: to-array.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

void **hlist_to_array(T list, void *end)
{
  int    n;
  void **array;
  int    i;

  n = hlist_length(list);

  array = malloc((n + 1) * sizeof(*array));
  if (array == NULL)
    return NULL;

  for (i = 0; i < n; i++)
  {
    array[i] = list->first;
    list = list->rest;
  }

  array[i] = end;
  return array;
}
