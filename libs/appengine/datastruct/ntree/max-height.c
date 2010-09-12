/* --------------------------------------------------------------------------
 *    Name: max-height.c
 * Purpose: N-ary tree
 * Version: $Id: max-height.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
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
