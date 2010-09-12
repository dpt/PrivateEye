/* --------------------------------------------------------------------------
 *    Name: unlink.c
 * Purpose: N-ary tree
 * Version: $Id: unlink.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

void ntree__unlink(ntree_t *t)
{
  assert(t);

  if (t->prev)
    t->prev->next = t->next;
  else if (t->parent)
    t->parent->children = t->next;

  t->parent = NULL;

  if (t->next)
  {
    t->next->prev = t->prev;
    t->next = NULL;
  }

  t->prev = NULL;
}
