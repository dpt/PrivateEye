/* --------------------------------------------------------------------------
 *    Name: max-height.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

int ntree__max_height(ntree_t *t)
{
  int      max;
  ntree_t *child;

  max = 0;

  if (!t)
    return max;

  for (child = t->children; child; child = child->next)
  {
    int h;

    h = ntree__max_height(child);
    if (h > max)
      max = h;
  }

  return max + 1;
}
