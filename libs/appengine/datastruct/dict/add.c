/* --------------------------------------------------------------------------
 *    Name: add.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/dict.h"

#include "impl.h"

enum
{
  LocMinimum = 4, /* minimum number of pool pointers to allocate */
  StrMinimum = 4, /* minimum number of pool pointers to allocate */
};

/* ----------------------------------------------------------------------- */

/* Ensure we have space for one more location struct. */
error dict__ensure_loc_space(dict_t *d)
{
  loc     *newpool;
  locpool *pool;

  assert(d);

  /* if there are pools allocated and there is space in the last allocated
   * pool, then there's space */

  if (d->l_used > 0 && d->locpools[d->l_used - 1].used + 1 <= (1 << d->log2locpoolsz))
    return error_OK;

  /* otherwise the last allocated pool is full or there are no pools
   * allocated */

  /* do we need to grow the pool list? */

  if (d->l_used == d->l_allocated)
  {
    size_t   newallocated;
    locpool *newpools;

    newallocated = d->l_used + 1;
    /* start with at least this many entries */
    if (newallocated < LocMinimum)
      newallocated = LocMinimum;
    /* subtract 1 to make it greater than or equal */
    newallocated = (size_t) power2gt(newallocated - 1);

    newpools = realloc(d->locpools, newallocated * sizeof(*newpools));
    if (newpools == NULL)
      return error_OOM;

    d->locpools    = newpools;
    d->l_allocated = newallocated;
  }

  /* allocate a new pool */

  newpool = malloc(sizeof(*newpool) << d->log2locpoolsz);
  if (newpool == NULL)
    return error_OOM;

  /* insert it into the list */

  pool = &d->locpools[d->l_used++];
  pool->locs = newpool;
  pool->used = 0;

  return error_OK;
}

/* Ensure we have space for 'len' more string bytes. */
/* Note: This basically identical to the above routine. */
error dict__ensure_str_space(dict_t *d, size_t len)
{
  char    *newpool;
  strpool *pool;

  assert(d);

  /* if there are pools allocated and there is space in the last allocated
   * pool, then there's space */

  if (d->s_used > 0 && d->strpools[d->s_used - 1].used + len <= (1 << d->log2strpoolsz))
    return error_OK;

  /* otherwise the last allocated pool is full or there are no pools
   * allocated */

  /* do we need to grow the pool list? */

  if (d->s_used == d->s_allocated)
  {
    size_t   newallocated;
    strpool *newpools;

    newallocated = d->s_used + 1;
    /* start with at least this many entries */
    if (newallocated < StrMinimum)
      newallocated = StrMinimum;
    /* subtract 1 to make it greater than or equal */
    newallocated = (size_t) power2gt(newallocated - 1);

    newpools = realloc(d->strpools, newallocated * sizeof(*newpools));
    if (newpools == NULL)
      return error_OOM;

    d->strpools    = newpools;
    d->s_allocated = newallocated;
  }

  /* allocate a new pool */

  newpool = malloc(sizeof(*newpool) << d->log2strpoolsz);
  if (newpool == NULL)
    return error_OOM;

  /* insert it into the list */

  pool = &d->strpools[d->s_used++];
  pool->strs = newpool;
  pool->used = 0;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error dict__add(dict_t *d, const char *string, dict_index *pidx)
{
  assert(d);
  assert(string);
  assert(pidx);

  return dict__add_with_len(d, string, strlen(string), pidx);
}

error dict__add_with_len(dict_t *d, const char *string, size_t len,
                         dict_index *pidx)
{
  error       err;
  dict_index  idx;
  locpool    *pend;
  locpool    *p;
  loc        *lend;
  loc        *l;
  int         i;

  assert(d);
  assert(string);

  idx = dict__index_with_len(d, string, len);
  if (idx >= 0) /* already present */
  {
    if (pidx)
      *pidx = idx;
    return error_DICT_NAME_EXISTS;
  }

  len++; /* lengths must account for a terminating NUL */

  /* Do we have a spare (previously deallocated) entry which can take this
   * data? It must be exactly the right size. Doing this mitigates part of
   * the delete-add-repeat unbounded growth problem but is work proportional
   * to the number of allocated ptrs. */

  pend = d->locpools + d->l_used;
  for (p = d->locpools; p < pend; p++)
  {
    lend = p->locs + p->used;
    for (l = p->locs; l < lend; l++)
      if (l->length == -((int) len)) /* careful of unsigned len */
        goto fillin;
  }

  /* if we're here we didn't find a block which was exactly the right size */

  /* ensure we have at least one spare location */
  err = dict__ensure_loc_space(d);
  if (err)
    return err;

  /* now we need to find spare space in a string block */

  /* We could just look at the tail end of the last-used string pool but it's
   * possible that the earlier pools have a suitably-sized free area at the
   * end of their allocated blocks so we'll scan through them all just in
   * case.
   */
  for (i = 0; i < d->s_used; i++)
    if ((1 << d->log2strpoolsz) - d->strpools[i].used >= len)
      break;

  if (i == d->s_used)
  {
    /* didn't find a suitable gap - need to allocate more string space */

    err = dict__ensure_str_space(d, len);
    if (err)
      return err;

    i = d->s_used - 1; /* the free space will be in the last pool */
  }

  /* allocate a new location */

  p = &d->locpools[d->l_used - 1];
  l = &p->locs[p->used++];

  l->ptr = d->strpools[i].strs + d->strpools[i].used;

  /* allocate string pool */

  d->strpools[i].used += len;

fillin:

  l->length = len;

  /* don't assume there's a terminator following 'string' */
  memcpy(l->ptr, string, len - 1);
  l->ptr[len - 1] = '\0'; /* explicity add a terminating NUL */

  if (pidx)
    *pidx = ((p - d->locpools) << d->log2locpoolsz) + (l - p->locs);

  return error_OK;
}
