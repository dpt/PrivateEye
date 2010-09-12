/* --------------------------------------------------------------------------
 *    Name: free.c
 * Purpose: N-ary tree
 * Version: $Id: free.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/ntree.h"

#include "impl.h"

void ntree__free(ntree_t *t)
{
  ntree_t *next;

  assert(t);

  for (; t; t = next)
  {
    next = t->next;

    if (t->children)
      ntree__free(t->children);

    free(t);
  }
}
