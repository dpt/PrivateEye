/* --------------------------------------------------------------------------
 *    Name: n-nodes.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"

#include "appengine/datastruct/ntree.h"

#include "impl.h"

static error ntree__count_func(ntree_t *t, void *arg)
{
  int *n;

  NOT_USED(t);

  n = arg;

  (*n)++;

  return error_OK;
}

int ntree__n_nodes(ntree_t *t)
{
  int n;

  n = 0;

  ntree__walk(t, ntree__WALK_IN_ORDER | ntree__WALK_ALL, 0,
              ntree__count_func, (void *) &n);

  return n;
}
