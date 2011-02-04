
#include "appengine/base/strings.h"

const char *str_leaf(const char *path)
{
  const char *p;
  const char *leaf;
  char c;

  p = leaf = path;
  do
  {
    c = *p++;
    if (c == ':' || c == '.')
      leaf = p; /* the char after */
  }
  while (c >= ' ');

  return leaf;
}
