/* --------------------------------------------------------------------------
 *    Name: hash.h
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

/* The interface presently forces you to malloc all keys, and values passed
 * in, yourself. */

#ifndef APPENGINE_HASH_H
#define APPENGINE_HASH_H

#include "appengine/base/errors.h"

#define T hash_t

typedef struct T T;

typedef unsigned int (hash__fn)(const void *a);
typedef int (hash__compare)(const void *a, const void *b);
typedef void (hash__destroy_key)(void *key);
typedef void (hash__destroy_value)(void *value);

error hash__create(int size, hash__fn *hash, hash__compare *compare,
                   hash__destroy_key *destroy_key,
                   hash__destroy_value *destroy_value, T **h);
void hash__destroy(T *h);

void *hash__lookup(T *h, const void *key);

error hash__insert(T *h, void *key, void *value);

void hash__remove(T *h, const void *key);

typedef int (hash__walk_callback)(const void *key, const void *value,
                                  void *arg);

int hash__walk(T *h, hash__walk_callback *cb, void *cbarg);

/* Walk a hash by continuation value.
 * Zero should be the initial continuation value. Return value is the next
 * continuation value.
 * Returns: -1 if invalid continuation given, 0 if no more elements */
int hash__walk_continuation(T           *h,
                            int          continuation,
                            const void **key,
                            const void **value);

#undef T

#endif /* APPENGINE_HASH_H */
