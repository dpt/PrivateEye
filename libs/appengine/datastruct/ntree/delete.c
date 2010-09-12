/* --------------------------------------------------------------------------
 *    Name: delete.c
 * Purpose: N-ary tree
 * Version: $Id: delete.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
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
