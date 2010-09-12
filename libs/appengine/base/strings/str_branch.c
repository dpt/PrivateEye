/* $Id: str_branch.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include "appengine/base/strings.h"

const char *str_branch(const char *path)
{
  static char buffer[256]; /* FIXME: Careful Now */

  str_n_cpy(buffer, path, str_leaf(path) - path - 1);

  return buffer;
}
