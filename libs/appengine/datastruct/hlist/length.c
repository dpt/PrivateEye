/* $Id: length.c,v 1.2 2008-08-05 22:05:05 dpt Exp $ */

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hlist.h"

#include "impl.h"

int hlist_length(T list)
{
  int n;

  for (n = 0; list; list = list->rest)
    n++;

  return n;
}
