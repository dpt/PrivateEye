/* --------------------------------------------------------------------------
 *    Name: next-sibling.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree_next_sibling(ntree_t *t)
{
  return t ? t->next : NULL;
}
