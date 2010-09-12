/* --------------------------------------------------------------------------
 *    Name: walk.c
 * Purpose: N-ary tree
 * Version: $Id: walk.c,v 1.3 2009-05-18 22:07:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/datastruct/ntree.h"

#include "impl.h"

static error walk_in_order(ntree_t *t, ntree__walk_flags flags, int depth,
                           ntree__walk_fn *fn, void *arg)
{
  error err;

  if (t->children)
  {
    ntree_t *sibling;
    ntree_t *next;

    depth--;

    if (depth)
    {
      /* process first child */
      err = walk_in_order(t->children, flags, depth, fn, arg);
      if (err)
        return err;
    }

    /* then next process the node itself */
    if (flags & ntree__WALK_BRANCHES)
    {
      err = fn(t, arg);
      if (err)
        return err;
    }

    if (depth)
    {
      /* finally process the remainder of the children */
      for (sibling = t->children->next; sibling; sibling = next)
      {
        next = sibling->next;
        err = walk_in_order(sibling, flags, depth, fn, arg);
        if (err)
          return err;
      }
    }
  }
  else
  {
    if (flags & ntree__WALK_LEAVES)
    {
      err = fn(t, arg);
      if (err)
        return err;
    }
  }

  return error_OK;
}

static error walk_pre_order(ntree_t *t, ntree__walk_flags flags, int depth,
                            ntree__walk_fn *fn, void *arg)
{
  error err;

  if (t->children)
  {
    /* process the node itself */
    if (flags & ntree__WALK_BRANCHES)
    {
      err = fn(t, arg);
      if (err)
        return err;
    }

    if (--depth)
    {
      ntree_t *sibling;
      ntree_t *next;

      /* finally process the children */
      for (sibling = t->children; sibling; sibling = next)
      {
        next = sibling->next;
        err = walk_pre_order(sibling, flags, depth, fn, arg);
        if (err)
          return err;
      }
    }
  }
  else
  {
    if (flags & ntree__WALK_LEAVES)
    {
      err = fn(t, arg);
      if (err)
        return err;
    }
  }

  return error_OK;
}

static error walk_post_order(ntree_t *t, ntree__walk_flags flags, int depth,
                             ntree__walk_fn *fn, void *arg)
{
  error err;

  if (t->children)
  {
    if (--depth)
    {
      ntree_t *sibling;
      ntree_t *next;

      /* process the children */
      for (sibling = t->children; sibling; sibling = next)
      {
        next = sibling->next;
        err = walk_post_order(sibling, flags, depth, fn, arg);
        if (err)
          return err;
      }
    }

    /* finally process the node itself */
    if (flags & ntree__WALK_BRANCHES)
    {
      err = fn(t, arg);
      if (err)
        return err;
    }
  }
  else
  {
    if (flags & ntree__WALK_LEAVES)
    {
      err = fn(t, arg);
      if (err)
        return err;
    }
  }

  return error_OK;
}

error ntree__walk(ntree_t *t, ntree__walk_flags flags, int max_depth,
                  ntree__walk_fn *fn, void *arg)
{
  error (*walker)(ntree_t *, ntree__walk_flags, int,
                  ntree__walk_fn *, void *);

  switch (flags & ntree__WALK_ORDER_MASK)
  {
  default:
  case ntree__WALK_IN_ORDER:
    walker = walk_in_order;
    break;
  case ntree__WALK_PRE_ORDER:
    walker = walk_pre_order;
    break;
  case ntree__WALK_POST_ORDER:
    walker = walk_post_order;
    break;
  }

  return walker(t, flags, max_depth, fn, arg);
}
