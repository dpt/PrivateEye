/* $Id: str_dup.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/appengine.h"
#include "appengine/base/strings.h"

char *str_dup(const char *str)
{
  char *new_str;

  new_str = malloc(str_len(str) + 1);
  if (new_str != NULL)
    str_cpy(new_str, str);

  return new_str;
}
