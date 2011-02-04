
#include "appengine/base/strings.h"

void str_term(char *s)
{
  while (*s >= ' ')
    s++;
  *s = '\0';
}
