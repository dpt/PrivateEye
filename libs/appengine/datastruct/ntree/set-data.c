/* --------------------------------------------------------------------------
 *    Name: set-data.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/ntree.h"

#include "impl.h"

void ntree__set_data(ntree_t *t, void *data)
{
  t->data = data;
}
