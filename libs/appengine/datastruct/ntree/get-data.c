/* --------------------------------------------------------------------------
 *    Name: get-data.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

void *ntree_get_data(ntree_t *t)
{
  return t->data;
}
