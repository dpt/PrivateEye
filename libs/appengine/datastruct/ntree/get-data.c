/* --------------------------------------------------------------------------
 *    Name: get-data.c
 * Purpose: N-ary tree
 * Version: $Id: get-data.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

void *ntree__get_data(ntree_t *t)
{
  return t->data;
}
