
#include "appengine/base/strings.h"

void str_cpy(char *to, const char *from)
{
  int c;

  do
  {
    c = *from++;
    if (c >= ' ')
      *to++ = c;
  }
  while (c >= ' ');
  *to = '\0';
}
