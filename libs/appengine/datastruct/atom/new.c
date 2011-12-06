/* --------------------------------------------------------------------------
 *    Name: new.c
 * Purpose: Atoms
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"

#include "appengine/datastruct/atom.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

/* Ensure we have space for one more location struct. */
error atom_ensure_loc_space(atom_set_t *s)
{
  loc     *newpool;
  locpool *pool;

  assert(s);

  /* if there are pools allocated and there is space in the last allocated
   * pool, then there's space */

  if (s->l_used > 0 && s->locpools[s->l_used - 1].used + 1 <= (1 << s->log2locpoolsz))
    return error_OK;

  /* otherwise the last allocated pool is full or there are no pools
   * allocated */

  /* do we need to grow the pool list? */

  if (s->l_used == s->l_allocated)
  {
    size_t   newallocated;
    locpool *newpools;

    newallocated = s->l_used + 1;
    /* start with at least this many entries */
    if (newallocated < LOCPTRMINSZ)
      newallocated = LOCPTRMINSZ;
    /* subtract 1 to make it greater than or equal */
    newallocated = (size_t) power2gt(newallocated - 1);

    newpools = realloc(s->locpools, newallocated * sizeof(*newpools));
    if (newpools == NULL)
      return error_OOM;

    s->locpools    = newpools;
    s->l_allocated = newallocated;
  }

  /* allocate a new pool */

  newpool = malloc(sizeof(*newpool) << s->log2locpoolsz);
  if (newpool == NULL)
    return error_OOM;

  /* insert it into the list */

  pool = &s->locpools[s->l_used++];
  pool->locs = newpool;
  pool->used = 0;

  return error_OK;
}

/* Ensure we have space for 'length' more block bytes. */
/* Note: This basically identical to the above routine, save for the types. */
error atom_ensure_blk_space(atom_set_t *s, size_t length)
{
  unsigned char *newpool;
  blkpool       *pool;

  assert(s);
  assert(length > 0);

  /* if there are pools allocated and there is space in the last allocated
   * pool, then there's space */

  if (s->b_used > 0 && s->blkpools[s->b_used - 1].used + length <= (1 << s->log2blkpoolsz))
    return error_OK;

  /* otherwise the last allocated pool is full or there are no pools
   * allocated */

  /* do we need to grow the pool list? */

  if (s->b_used == s->b_allocated)
  {
    size_t   newallocated;
    blkpool *newpools;

    newallocated = s->b_used + 1;
    /* start with at least this many entries */
    if (newallocated < BLKPTRMINSZ)
      newallocated = BLKPTRMINSZ;
    /* subtract 1 to make it greater than or equal */
    newallocated = (size_t) power2gt(newallocated - 1);

    newpools = realloc(s->blkpools, newallocated * sizeof(*newpools));
    if (newpools == NULL)
      return error_OOM;

    s->blkpools    = newpools;
    s->b_allocated = newallocated;
  }

  /* allocate a new pool */

  newpool = malloc(sizeof(*newpool) << s->log2blkpoolsz);
  if (newpool == NULL)
    return error_OOM;

  /* insert it into the list */

  pool = &s->blkpools[s->b_used++];
  pool->blks = newpool;
  pool->used = 0;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error atom_new(atom_set_t *s, const unsigned char *block, size_t length,
               atom_t *patom)
{
  error    err;
  atom_t   atom;
  locpool *pend;
  locpool *p;
  loc     *lend;
  loc     *l;
  int      i;

  assert(s);
  assert(block);
  assert(length > 0);
  assert(patom);

  atom = atom_for_block(s, block, length);
  if (atom >= 0) /* already present */
  {
    if (patom)
      *patom = atom;
    return error_ATOM_NAME_EXISTS;
  }

  /* Do we have a spare (previously deallocated) entry which can take this
   * data? It must be exactly the right size. Doing this mitigates part of
   * the delete-add-repeat unbounded growth problem but is work proportional
   * to the number of allocated ptrs. */

  pend = s->locpools + s->l_used;
  for (p = s->locpools; p < pend; p++)
  {
    lend = p->locs + p->used;
    for (l = p->locs; l < lend; l++)
      if (l->length == -((int) length)) /* careful of unsigned len */
        goto fillin;
  }

  /* if we're here we didn't find a block which was exactly the right size */

  /* ensure we have at least one spare location */
  err = atom_ensure_loc_space(s);
  if (err)
    return err;

  /* now we need to find spare space in a pool */

  /* We could just look at the tail end of the last-used block pool but it's
   * possible that the earlier pools have a suitably-sized free area at the
   * end of their allocated blocks so we'll scan through them all just in
   * case.
   */
  for (i = 0; i < s->b_used; i++)
    if ((1 << s->log2blkpoolsz) - s->blkpools[i].used >= length)
      break;

  if (i == s->b_used)
  {
    /* didn't find a suitable gap - need to allocate more block space */

    err = atom_ensure_blk_space(s, length);
    if (err)
      return err;

    i = s->b_used - 1; /* the free space will be in the last pool */
  }

  /* allocate a new location */

  p = &s->locpools[s->l_used - 1];
  l = &p->locs[p->used++];

  l->ptr = s->blkpools[i].blks + s->blkpools[i].used;

  /* allocate block pool */

  s->blkpools[i].used += length;

fillin:

  l->length = length;

  memcpy(l->ptr, block, length);

  if (patom)
    *patom = ((p - s->locpools) << s->log2locpoolsz) + (l - p->locs);

  return error_OK;
}
