/* --------------------------------------------------------------------------
 *    Name: lookup.c
 * Purpose: Hash
 * Version: $Id: lookup.c,v 1.3 2010-01-13 18:01:18 dpt Exp $
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
