
#include <ctype.h>
#include <stddef.h>

#include "appengine/base/strings.h"

int strnncasecmp(const char *s1, size_t n1, const char *s2, size_t n2)
{
  const unsigned char *u1, *u2;

  u1 = (const unsigned char *) s1;
  u2 = (const unsigned char *) s2;

  for (; n1 > 0 && n2 > 0; n1--, n2--)
  {
    unsigned int c1, c2;

    c1 = tolower(*s1);
    c2 = tolower(*s2);
    if (c1 != c2)
      return c1 - c2;

    s1++;
    s2++;
  }

  return n1 - n2;
}
