
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

T hlist_pop(T list, void **x)
{
  if (list)
  {
    T head = list->rest;
    if (x)
      *x = list->first;
    free(list);
    return head;
  }
  else
  {
    return list;
  }
}
