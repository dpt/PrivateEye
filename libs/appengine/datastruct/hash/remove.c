/* --------------------------------------------------------------------------
 *    Name: remove.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/hash.h"

#include "impl.h"

void hash__remove_node(hash_t *h, node **n)
{
  node *doomed;

  doomed = *n;

  *n = doomed->next;

  h->destroy_key(doomed->key);
  h->destroy_value(doomed->value);

  free(doomed);
}

void hash__remove(hash_t *h, const void *key)
{
  node **n;

  n = hash__lookup_node(h, key);
  hash__remove_node(h, n);
}
