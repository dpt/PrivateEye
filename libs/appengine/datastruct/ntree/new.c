/* --------------------------------------------------------------------------
 *    Name: new.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"

#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__new(ntree_t **t)
{
  ntree_t *n;

  n = calloc(1, sizeof(*n));
  if (n == NULL)
    return error_OOM;

  *t = n;

  return error_OK;
}
