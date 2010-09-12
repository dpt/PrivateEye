/* --------------------------------------------------------------------------
 *    Name: last-child.c
 * Purpose: N-ary tree
 * Version: $Id: last-child.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
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
