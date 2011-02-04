
#include "appengine/base/strings.h"

const char *str_branch(const char *path)
{
  static char buffer[256]; /* FIXME: Careful Now */

  str_n_cpy(buffer, path, str_leaf(path) - path - 1);

  return buffer;
}
