/* $Id: str_n_cpy.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include "appengine/base/strings.h"

void str_n_cpy(char *to, const char *from, int size)
{
  int c;

  /* - size is length of string excluding terminator
   * - always terminates the output string */

  if (size == 0)
    return;

  do
  {
    c = *from++;
    if (c >= ' ')
      *to++ = c;
  }
  while (c >= ' ' && --size);

  *to = '\0';
}
