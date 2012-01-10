/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/primes.h"

#include "appengine/datastruct/hash.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

/* Basic string hash. Retained for reference. */
/*static unsigned int string_hash(const void *a)
{
  const char  *s = a;
  unsigned int h;

  h = 0;
  while (*s)
    h = h * 37 + *s++;

  return h;
}*/

/* Fowler/Noll/Vo FNV-1 hash */
static unsigned int string_hash(const void *a)
{
  const char  *s = a;
  unsigned int h;

  h = 0x811c9dc5;
  while (*s)
  {
    h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
    h ^= *s++;
  }

  return h;
}

static int string_compare(const void *a, const void *b)
{
  const char *sa = a;
  const char *sb = b;

  return strcmp(sa, sb);
}

static void string_destroy(void *string)
{
  free(string);
}

/* ----------------------------------------------------------------------- */

error hash__create(int                  nbins,
                   hash__fn            *fn,
                   hash__compare       *compare,
                   hash__destroy_key   *destroy_key,
                   hash__destroy_value *destroy_value,
                   hash_t             **ph)
{
  hash_t *h;
  node  **bins;

  h = malloc(sizeof(*h));
  if (h == NULL)
    return error_OOM;

  nbins = prime_nearest(nbins);

  bins = calloc(nbins, sizeof(*h->bins));
  if (bins == NULL)
    return error_OOM;

  /* the default routines handle strings */

  h->hash_fn       = fn            ? fn            : string_hash;
  h->compare       = compare       ? compare       : string_compare;
  h->destroy_key   = destroy_key   ? destroy_key   : string_destroy;
  h->destroy_value = destroy_value ? destroy_value : string_destroy;
  h->nbins         = nbins;
  h->bins          = bins;

  *ph = h;

  return error_OK;
}
