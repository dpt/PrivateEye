/* --------------------------------------------------------------------------
 *    Name: insert-after.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__insert_after(ntree_t *parent, ntree_t *sibling, ntree_t *node)
{
  NOT_USED(parent);
  NOT_USED(sibling);
  NOT_USED(node);

  assert("NYI" == NULL);

  return error_OK;
}
