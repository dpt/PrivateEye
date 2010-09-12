/* --------------------------------------------------------------------------
 *    Name: copy.c
 * Purpose: N-ary tree
 * Version: $Id: copy.c,v 1.3 2009-05-18 22:07:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree__copy(ntree_t *t, ntree__copy_fn *fn, void *arg, ntree_t **new_t)
{
  error    err;
  void    *data;
  ntree_t *new_node;
  ntree_t *child;

  if (!t)
    return NULL;

  err = fn(t->data, arg, &data);
  if (err)
    return err;

  err = ntree__new(&new_node);
  if (err)
    return err;

  ntree__set_data(new_node, data);

  /* we walk the list of children backwards
   * so we can prepend, which takes constant time */

  for (child = ntree__last_child(t); child; child = child->prev)
  {
    ntree_t *new_child;

    err = ntree__copy(child, fn, arg, &new_child);
    if (err)
      return err;

    err = ntree__prepend(new_node, new_child);
    if (err)
      return err;
  }

  *new_t = new_node;

  return err;
}
