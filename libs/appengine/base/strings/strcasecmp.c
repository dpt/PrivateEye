
#include <ctype.h>
#include <stddef.h>

#include "appengine/base/strings.h"

int strcasecmp(const char *s1, const char *s2)
{
  const unsigned char *u1, *u2;

  u1 = (const unsigned char *) s1;
  u2 = (const unsigned char *) s2;

  for (;;)
  {
    unsigned int c1, c2;

    c1 = *s1;
    c2 = *s2;

    if (c1 == '\0' || c2 == '\0')
      return c1 - c2;

    c1 = tolower(c1);
    c2 = tolower(c2);
    if (c1 != c2)
      return c1 - c2;

    s1++;
    s2++;
  }
}
