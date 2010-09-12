/* $Id: str_len.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include "appengine/base/strings.h"

int str_len(const char *s)
{
  const char *p;

  for (p = s; *p >= ' '; p++)
    ;

  return p - s;
}
