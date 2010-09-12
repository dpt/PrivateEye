/* --------------------------------------------------------------------------
 *    Name: prepend.c
 * Purpose: N-ary tree
 * Version: $Id: prepend.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__prepend(ntree_t *parent, ntree_t *node)
{
  return ntree__insert_before(parent, parent->children, node);
}
