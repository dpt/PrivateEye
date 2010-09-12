/* $Id: str_n_dup.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/appengine.h"
#include "appengine/base/strings.h"

char *str_n_dup(const char *str, int size)
{
  char *new_str;

  new_str = malloc(size);
  if (new_str != NULL)
    str_n_cpy(new_str, str, size);

  return new_str;
}
