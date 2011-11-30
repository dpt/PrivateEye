/* --------------------------------------------------------------------------
 *    Name: index.c
 * Purpose: Dictionary
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
  locpool *pend;
  locpool *p;

  assert(d);
  assert(string);

  /* linear search every locpool until found */

  pend = d->locpools + d->l_used;
  for (p = d->locpools; p < pend; p++)
  {
    loc *lend;
    loc *l;

    /* don't assume there's a terminator following 'string' */

    lend = p->locs + p->used;
    for (l = p->locs; l < lend; l++)
      if (l->length - 1 == len && memcmp(l->ptr, string, len) == 0)
        return ((p - d->locpools) << d->log2locpoolsz) + (l - p->locs);
  }

  return -1;
}
