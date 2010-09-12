/* --------------------------------------------------------------------------
 *    Name: nth-child.c
 * Purpose: N-ary tree
 * Version: $Id: nth-child.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree__nth_child(ntree_t *t, int n)
{
  t = t->children;
  if (t)
    while (n-- > 0 && t)
      t = t->next;

  return t;
}
