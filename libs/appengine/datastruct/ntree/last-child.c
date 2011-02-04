/* --------------------------------------------------------------------------
 *    Name: last-child.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree__last_child(ntree_t *t)
{
  t = t->children;
  if (t)
    while (t->next)
      t = t->next;

  return t;
}
