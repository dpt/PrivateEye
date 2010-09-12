/* --------------------------------------------------------------------------
 *    Name: rename.c
 * Purpose: Dictionary
 * Version: $Id: rename.c,v 1.5 2009-05-25 22:06:26 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <string.h>

#include "oslib/types.h"

#include "appengine/datastruct/dict.h"

#include "impl.h"

error dict__rename(dict_t *d, dict_index index, const char *string)
{
  error err;
  int   newlen;
  int   i;
  int   oldlen;
  char *ds;

  assert(d);
  assert(string);

  if ((unsigned int) index >= d->d_used)
    return error_DICT_OUT_OF_RANGE;

  newlen = strlen(string);

  /* does the new name already exist in the dict? */

  i = dict__index_with_len(d, string, newlen);
  if (i >= 0)
    /* renaming an index to its existing name is just ignored */
    return (i == index) ? error_OK : error_DICT_NAME_EXISTS;

  oldlen = d->data[index].length - 1; /* uncount terminator */

  if (newlen > oldlen)
  {
    /* need (newlen - oldlen) bytes spare */
    err = dict__ensure_string_space(d, newlen - oldlen);
    if (err)
      return err;
  }

  ds = d->strings + d->data[index].offset;

  if (newlen != oldlen)
    memmove(ds + newlen + 1,
            ds + oldlen + 1,
            d->s_used - (d->data[index].offset + oldlen + 1));

  memcpy(ds, string, newlen);
  ds[newlen] = '\0';

  if (newlen != oldlen)
    d->s_used += newlen - oldlen;

  d->data[index].length = newlen + 1; /* count terminator */

  /* shuffle all the affected pointers forward */

  for (i = 0; i < d->d_used; i++)
    if (d->strings + d->data[i].offset > ds)
      d->data[i].offset += newlen - oldlen;

  return error_OK;
}
