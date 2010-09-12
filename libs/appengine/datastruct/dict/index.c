/* --------------------------------------------------------------------------
 *    Name: index.c
 * Purpose: Dictionary
 * Version: $Id: index.c,v 1.3 2009-05-24 23:39:32 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "appengine/datastruct/dict.h"

#include "impl.h"

dict_index dict__index(dict_t *d, const char *string)
{
  assert(d);
  assert(string);

  return dict__index_and_len(d, string, NULL);
}

dict_index dict__index_and_len(dict_t *d, const char *string, size_t *len)
{
  size_t l;

  assert(d);
  assert(string);

  l = strlen(string);

  if (len)
    *len = l;

  return dict__index_with_len(d, string, l);
}

dict_index dict__index_with_len(dict_t *d, const char *string, size_t len)
{
  data *end;
  data *e;

  assert(d);
  assert(string);

  /* don't assume there's a terminator following 'string' */

  /* linear search */

  end = d->data + d->d_used;
  for (e = d->data; e < end; e++)
    if (e->length - 1 == len &&
        memcmp(d->strings + e->offset, string, len) == 0)
      return e - d->data;

  return -1;
}
