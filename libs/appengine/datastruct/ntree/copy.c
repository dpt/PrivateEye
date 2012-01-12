/* --------------------------------------------------------------------------
 *    Name: copy.c
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/datastruct/ntree.h"

#include "impl.h"

error ntree_copy(ntree_t *t, ntree_copy_fn *fn, void *arg, ntree_t **new_t)
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

  err = ntree_new(&new_node);
  if (err)
    return err;

  ntree_set_data(new_node, data);

  /* we walk the list of children backwards
   * so we can prepend, which takes constant time */

  for (child = ntree_last_child(t); child; child = child->prev)
  {
    ntree_t *new_child;

    err = ntree_copy(child, fn, arg, &new_child);
    if (err)
      return err;

    err = ntree_prepend(new_node, new_child);
    if (err)
      return err;
  }

  *new_t = new_node;

  return err;
}
