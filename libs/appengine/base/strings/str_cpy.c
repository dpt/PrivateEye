/* $Id: str_cpy.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

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
