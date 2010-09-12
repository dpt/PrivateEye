/* $Id: strcasecmp.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include <ctype.h>

#include "appengine/base/strings.h"

int strcasecmp(const char *s1, const char *s2)
{
  while (tolower(*s1) == tolower(*s2++))
    if (*s1++ == '\0')
      return 0;

  return (tolower(*s1) < tolower(*--s2)) ? -1 : +1;
}
