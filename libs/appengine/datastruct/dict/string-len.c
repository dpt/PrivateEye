/* --------------------------------------------------------------------------
 *    Name: string-len.c
 * Purpose: Dictionary
 * Version: $Id: string-len.c,v 1.3 2009-05-24 23:39:32 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>

#include "appengine/datastruct/dict.h"

#include "impl.h"

const char *dict__string_and_len(dict_t *d, size_t *len, dict_index index)
{
  assert(d);

  if (!d->strings)
    return NULL; /* empty dict */

  if ((unsigned int) index >= d->d_used)
    return NULL; /* out of range */

  if (d->data[index].length < 0)
    return NULL; /* deleted */

  if (len)
    *len = d->data[index].length - 1; /* -1 for terminator */

  return d->strings + d->data[index].offset;
}
