/* --------------------------------------------------------------------------
 *    Name: dict.h
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

/* Maintains a set of strings. Each string is assigned a number. */

#ifndef APPENGINE_DICT_H
#define APPENGINE_DICT_H

#include <stddef.h>

#include "appengine/base/errors.h"

#define T dict_t

typedef struct T T;

T *dict__create(void);
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
