
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

T hlist_append(T list, T tail)
{
  T *p = &list;

  while (*p)
    p = &(*p)->rest;

  *p = tail;

  return list;
}
