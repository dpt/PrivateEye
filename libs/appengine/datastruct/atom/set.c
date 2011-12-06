/* --------------------------------------------------------------------------
 *    Name: set.c
 * Purpose: Atoms
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <string.h>

#include "appengine/datastruct/atom.h"

#include "impl.h"

error atom_set(atom_set_t          *s,
               atom_t               a,
               const unsigned char *block,
               size_t               length)
{
  error   err;
  atom_t  newa;
  loc    *p, *q;
  loc     t;

  assert(s);
  assert(a != atom_NOT_FOUND);
  assert(block);
  assert(length > 0);

  if (!s->locpools)
    return error_ATOM_SET_EMPTY;

  assert(s->l_used >= 1);

  if (!ATOMVALID(a))
    return error_ATOM_OUT_OF_RANGE;

  err = atom_new(s, block, length, &newa);
  if (err == error_ATOM_NAME_EXISTS && newa == a)
    /* setting an atom to its existing data is ignored */
    return error_OK;
  else if (err)
    return err;

  atom_delete(s, a);

  /* now transpose old and new atoms */

  p = &ATOMLOC(a);
  q = &ATOMLOC(newa);

  t  = *p;
  *p = *q;
  *q = t;

  return error_OK;
}
