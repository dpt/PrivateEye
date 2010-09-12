/* --------------------------------------------------------------------------
 *    Name: depth.c
 * Purpose: N-ary tree
 * Version: $Id: depth.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
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
