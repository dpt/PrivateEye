/* --------------------------------------------------------------------------
 *    Name: hash.h
 * Purpose: Interface of Hash library
 * ----------------------------------------------------------------------- */

/**
 * \file Hash (interface).
 *
 * Hash is an associative array.
 *
 * The interface presently forces you to malloc all keys, and values passed
 * in, yourself.
 */

#ifndef APPENGINE_HASH_H
#define APPENGINE_HASH_H

#include "appengine/base/errors.h"

#define T hash_t

typedef struct T T;

/* ----------------------------------------------------------------------- */

/**
 * A function called to hash the specified key.
 */
typedef unsigned int (hash_fn)(const void *a);

/**
 * A function called to compare the two specified keys.
 */
typedef int (hash_compare)(const void *a, const void *b);

/**
 * A function called to destroy the specified key.
 */
typedef void (hash_destroy_key)(void *key);

/**
 * A function called to destroy the specified value.
 */
typedef void (hash_destroy_value)(void *value);

/**
 * Create a hash.
 *
 * \param      nbins         Suggested number of hash bins to allocate.
 * \param      fn            Function to hash keys.
 * \param      compare       Function to compare keys.
 * \param      destroy_key   Function to destroy a key.
 * \param      destroy_value Function to destroy a value.
 * \param[out] hash          Created hash.
 *
 * \return Error indication.
 */
error hash_create(int                  nbins,
                   hash_fn            *fn,
                   hash_compare       *compare,
                   hash_destroy_key   *destroy_key,
                   hash_destroy_value *destroy_value,
                   T                  **hash);

/**
 * Destroy a hash.
 *
 * \param doomed Hash to destroy.
 */
void hash_destroy(T *doomed);

/* ----------------------------------------------------------------------- */

/**
 * Return the value associated with the specified key.
 *
 * \param hash Hash.
 * \param key  Key to look up.
 *
 * \return Value associated with the specified key.
 */
void *hash_lookup(T *hash, const void *key);

/**
 * Insert the specified key:value pair into the hash.
 *
 * The hash takes ownership of the key and value pointers. It will call the
 * destroy functions passed to hash_create when the keys and values are to
 * be destroyed.
 *
 * \param hash  Hash.
 * \param key   Key to insert.
 * \param value Associated value.
 *
 * \return Error indication.
 */
error hash_insert(T *hash, void *key, void *value);

/**
 * Remove the specified key from the hash.
 *
 * \param hash Hash.
 * \param key  Key to remove.
 */
void hash_remove(T *hash, const void *key);

/* ----------------------------------------------------------------------- */

/**
 * A function called for every key:value pair in the hash.
 *
 * Return a negative value to halt the walk operation.
 */
typedef int (hash_walk_callback)(const void *key,
                                  const void *value,
                                  void       *opaque);

/**
 * Walk the hash, calling the specified routine for every element.
 *
 * \param hash   Hash.
 * \param cb     Callback routine.
 * \param opaque Opaque pointer to pass to callback routine.
 *
 * \return The negative value returned from the callback, or zero for
 * success.
 */
int hash_walk(T *hash, hash_walk_callback *cb, void *opaque);

/* ----------------------------------------------------------------------- */

/**
 * Walk the hash, returning each element in turn.
 *
 * \param      hash         Hash.
 * \param      continuation Continuation value. Zero for the initial call.
 * \param[out] key          Pointer to receive key.
 * \param[out] value        Pointer to receive value.
 *
 * \return Next continuation value.
 * \retval  0 No more elements.
 * \retval -1 Invalid continuation value.
 */
int hash_walk_continuation(T           *hash,
                            int          continuation,
                            const void **key,
                            const void **value);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_HASH_H */
