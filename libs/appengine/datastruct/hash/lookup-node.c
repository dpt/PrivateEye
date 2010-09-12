/* --------------------------------------------------------------------------
 *    Name: lookup-node.c
 * Purpose: Hash
 * Version: $Id: lookup-node.c,v 1.3 2010-01-13 18:01:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/datastruct/hash.h"

#include "impl.h"

node **hash__lookup_node(hash_t *h, const void *key)
{
  int    hash;
  node **n;

  hash = h->hash_fn(key) % h->nbins;
  for (n = &h->bins[hash]; *n != NULL; n = &(*n)->next)
    if (h->compare(key, (*n)->key) == 0)
      break;

  return n;
}
