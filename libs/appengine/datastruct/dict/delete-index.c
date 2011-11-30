/* --------------------------------------------------------------------------
 *    Name: delete-index.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <string.h>

#include "appengine/datastruct/dict.h"

#include "impl.h"

void dict__delete_index(dict_t *d, dict_index idx)
{
  int *plength;
  int  length;

  assert(d);

  if (!d->locpools)
    return; /* empty dict */

  assert(d->l_used >= 1);

  if (!VALID(idx))
    return; /* out of range */

  plength = &LENGTH(idx);
  length  = *plength;
  if (length < 0)
    return; /* deleted */

#ifndef NDEBUG
  /* scribble over the deleted copy */
  memset(PTR(idx), 'x', length - 1);
#endif

  /* flip the sign of the length */
  /* this leaves the data in place but inaccessible */

  *plength = -length;
}
