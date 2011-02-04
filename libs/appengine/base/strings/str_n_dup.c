
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
