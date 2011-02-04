
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

T hlist_push(T list, void *x)
{
  T p;

  p = malloc(sizeof(*p));

  p->first = x;
  p->rest = list;

  return p;
}
