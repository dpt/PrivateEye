/* --------------------------------------------------------------------------
 *    Name: prev-sibling.c
 * Purpose: N-ary tree
 * Version: $Id: prev-sibling.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

ntree_t *ntree__prev_sibling(ntree_t *t)
{
  return t ? t->prev : NULL;
}
