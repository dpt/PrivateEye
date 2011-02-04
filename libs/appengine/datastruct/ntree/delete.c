/* --------------------------------------------------------------------------
 *    Name: delete.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

void ntree__delete(ntree_t *t)
{
  assert(t);

  if (!IS_ROOT(t))
    ntree__unlink(t);

  ntree__free(t);
}
