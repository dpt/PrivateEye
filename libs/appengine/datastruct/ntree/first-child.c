/* --------------------------------------------------------------------------
 *    Name: first-child.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree__first_child(ntree_t *t)
{
  return t ? t->children : NULL;
}
