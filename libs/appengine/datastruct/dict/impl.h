/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#ifndef DICT_IMPL_H
#define DICT_IMPL_H

#include <stdlib.h>

#include "appengine/datastruct/dict.h"

/* The table of strings retains terminators so that the 'get string' method
 * can just return a static pointer into that data. */

typedef struct data
{
  int offset; /* offset of string (offset < 0 => deleted) */
  int length; /* length of string, including terminator */
}
data;

struct dict_t
{
  char *strings; /* table of strings (including terminators) */
  int   s_used;
  int   s_allocated;

  data *data;
  int   d_used;
  int   d_allocated;
};

error dict__add_with_len(dict_t *d, const char *string, size_t len,
                         dict_index *index);

/* dict_index but returning the length of 'string' (from strlen) */
dict_index dict__index_and_len(dict_t *d, const char *string, size_t *len);

/* dict_index but taking the length of 'string' */
dict_index dict__index_with_len(dict_t *d, const char *string, size_t len);

error dict__ensure_string_space(dict_t *d, size_t len);
error dict__ensure_data_space(dict_t *d, size_t n);

#endif /* DICT_IMPL_H */
