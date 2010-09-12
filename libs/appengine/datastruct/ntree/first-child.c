/* --------------------------------------------------------------------------
 *    Name: first-child.c
 * Purpose: N-ary tree
 * Version: $Id: first-child.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree__first_child(ntree_t *t)
{
  return t ? t->children : NULL;
}
