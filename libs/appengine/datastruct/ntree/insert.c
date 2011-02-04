/* --------------------------------------------------------------------------
 *    Name: insert.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__insert(ntree_t *parent, int where, ntree_t *node)
{
  ntree_t *sibling;

  if (where > 0)
    sibling = ntree__nth_child(parent, where);
  else if (where == 0) /* prepend */
    sibling = parent->children;
  else /* append */
    sibling = NULL;

  return ntree__insert_before(parent, sibling, node);
}
