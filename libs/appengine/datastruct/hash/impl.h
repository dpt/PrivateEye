/* --------------------------------------------------------------------------
 *    Name: impl.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_HASH_IMPL_H
#define APPENGINE_HASH_IMPL_H

#include "appengine/datastruct/hash.h"

typedef struct node
{
  struct node         *next;
  void                *key;
  void                *value;
}
node;

struct hash_t
{
  hash__fn            *hash_fn;
  hash__compare       *compare;
  hash__destroy_key   *destroy_key;
  hash__destroy_value *destroy_value;
  int                  nbins;
  node               **bins;
};

node **hash__lookup_node(hash_t *h, const void *key);
void hash__remove_node(hash_t *h, node **n);

#endif /* APPENGINE_HASH_IMPL_H */
