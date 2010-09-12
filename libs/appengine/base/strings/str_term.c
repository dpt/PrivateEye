/* $Id: str_term.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include "appengine/base/strings.h"

void str_term(char *s)
{
  while (*s >= ' ')
    s++;
  *s = '\0';
}
