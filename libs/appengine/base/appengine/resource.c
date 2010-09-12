/* $Id: resource.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include <stdlib.h>

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"

/*
 * Expect this to only be called once per task.  This means buffer[] can
 * and should remain static, so the client can refer to it throughout the
 * duration of the task.
 *
 * It won't be released at shutdown, however.
 */

const char *resource_locate(const char *var)
{
  const char *path;

  EC(xappengine_resource_op_locate(var, &path));
  /* path now points to a CR terminated string */

  return str_dup(path);
}
