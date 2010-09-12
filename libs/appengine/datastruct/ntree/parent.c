/* --------------------------------------------------------------------------
 *    Name: parent.c
 * Purpose: N-ary tree
 * Version: $Id: parent.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree__parent(ntree_t *t)
{
  return t ? t->parent : NULL;
}
