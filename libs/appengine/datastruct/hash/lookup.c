/* --------------------------------------------------------------------------
 *    Name: lookup.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/hash.h"

#include "impl.h"

void *hash__lookup(hash_t *h, const void *key)
{
  node **n;

  n = hash__lookup_node(h, key);

  return (*n != NULL) ? (*n)->value : NULL;
}
