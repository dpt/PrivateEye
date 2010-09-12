/* --------------------------------------------------------------------------
 *    Name: insert-before.c
 * Purpose: N-ary tree
 * Version: $Id: insert-before.c,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__insert_before(ntree_t *parent, ntree_t *sibling, ntree_t *node)
{
  assert(node->parent == NULL);

  node->parent = parent;

  if (sibling)
  {
    if (sibling->prev)
    {
      node->prev = sibling->prev;
      node->prev->next = node;
      node->next = sibling;
      sibling->prev = node;
    }
    else
    {
      parent->children = node; // or node->parent->children = node;
      node->next = sibling;
      sibling->prev = node;
    }
  }
  else
  {
    /* not given a sibling */

    if (parent->children)
    {
      /* insert at end */
      sibling = parent->children;
      while (sibling->next)
        sibling = sibling->next;
      node->prev = sibling;
      sibling->next = node;
    }
    else
    {
      parent->children = node;
    }
  }

  return error_OK;
}
