
#include "appengine/base/strings.h"

int str_len(const char *s)
{
  const char *p;

  for (p = s; *p >= ' '; p++)
    ;

  return p - s;
}
