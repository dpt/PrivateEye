/* --------------------------------------------------------------------------
 *    Name: string-len.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>

#include "appengine/datastruct/dict.h"

#include "impl.h"

const char *dict__string_and_len(dict_t *d, size_t *len, dict_index idx)
{
  int length;

  assert(d);

  if (!d->locpools)
    return NULL; /* empty dict */

  assert(d->l_used >= 1);

  if (!VALID(idx))
    return NULL; /* out of range */

  length = LENGTH(idx);
  if (length < 0)
    return NULL; /* deleted */

  if (len)
    *len = length - 1; /* -1 to account for terminator */

  return PTR(idx);
}
