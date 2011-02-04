/* --------------------------------------------------------------------------
 *    Name: prepend.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__prepend(ntree_t *parent, ntree_t *node)
{
  return ntree__insert_before(parent, parent->children, node);
}
