/* --------------------------------------------------------------------------
 *    Name: dict.h
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

/* A dict maintains a set of strings. Each unique string is assigned an
 * index. */

/* Internally, strings are packed into fixed-length blocks. An additional
 * fixed-length block holds (string pointer, length) pairs.
 *
 * Blocks are never freed. Once space is allocated in the block for a string,
 * that region is allocated until it is deleted. The pointer returned by
 * dict__string is available for permanent use until the string is deleted,
 * or renamed. This allows a dict to hold strings which are shared by other
 * data structures, e.g. a hash, without needing to be concerned that the
 * block may move. (Older versions of dict had a single block and allowed the
 * block to move in memory when space was exhausted).
 *
 * New blocks are only ever allocated from spare space at the end of a block.
 * When new strings are inserted into a dict we search the blocks for an
 * exact-size spare entry and use that if one is available.
 *
 * Strings cannot presently exceed the size of a fixed-length block.
 */

#ifndef APPENGINE_DICT_H
#define APPENGINE_DICT_H

#include <stddef.h>

#include "appengine/base/errors.h"

#define T dict_t

typedef struct T T;

T *dict__create(void);
/* locpoolsz - how many entries you expect to store */
/* strpoolsz - total size of strings you expect to store */
T *dict__create_tuned(size_t locpoolsz, size_t strpoolsz);
void dict__destroy(T *d);

typedef int dict_index;

/* if the string already exists in the dict then dict__add will return
 * error_DICT_NAME_EXISTS, and 'index' will be the index of the existing
 * copy. */
error dict__add(T *d, const char *string, dict_index *index);
void dict__delete(T *d, const char *string);
void dict__delete_index(T *d, dict_index index);
error dict__rename(T *d, dict_index index, const char *string);

const char *dict__string(T *d, dict_index index);
const char *dict__string_and_len(T *d, size_t *len, dict_index index);
dict_index dict__index(T *d, const char *string);

#undef T

#endif /* APPENGINE_DICT_H */
