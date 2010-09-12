/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: N-ary tree
 * Version: $Id: impl.h,v 1.2 2008-08-05 22:05:05 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef NTREE_IMPL_H
#define NTREE_IMPL_H

#include <stdlib.h>

#include "appengine/datastruct/ntree.h"

struct ntree_t
{
  ntree_t *next;
  ntree_t *prev;
  ntree_t *parent;
  ntree_t *children;
  void    *data;
};

#define IS_ROOT(t) ((t)->parent == NULL && \
                    (t)->prev   == NULL && \
                    (t)->next   == NULL)

#endif /* NTREE_IMPL_H */
