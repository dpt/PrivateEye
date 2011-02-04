/* --------------------------------------------------------------------------
 *    Name: depth.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

int ntree__depth(ntree_t *t)
{
  int d;

  d = 0;

  while (t)
  {
    d++;
    t = t->parent;
  }

  return d;
}
