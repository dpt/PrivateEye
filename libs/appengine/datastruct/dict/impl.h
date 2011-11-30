/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#ifndef DICT_IMPL_H
#define DICT_IMPL_H

#include <stdlib.h>

#include "appengine/datastruct/dict.h"

#define LOG2LOCPOOLSZ 5 /* default: 32 locs per pool */
#define LOG2STRPOOLSZ 9 /* default: 512 bytes per pool */

/* Convert an index into a loc. */
#define INDEXTOLOC(i) d->locpools[(i) >> d->log2locpoolsz].locs[(i) & ((1 << d->log2locpoolsz) - 1)]
#define PTR(i) (INDEXTOLOC(i).ptr)
#define LENGTH(i) (INDEXTOLOC(i).length)

/* Returns valid if the specified index corresponds to an allocated (but not
 * necessarily used) index. */
#define VALID(i) ((unsigned int) i < ((d->l_used - 1) << d->log2locpoolsz) + d->locpools[d->l_used - 1].used)

/* Stores the location and length of a string. */
typedef struct loc
{
  char *ptr;    /* pointer to string */
  int   length; /* length of string, including terminator (-ve if deallocated) */
}
loc;

/* Stores the location and used count of a location pool. */
typedef struct locpool
{
  loc *locs; /* table of loc structs */
  int  used;
}
locpool;

/* Stores the location and used count of a string pool.
 * The table of strings retains terminators so that the 'get string' method
 * can just return a static pointer into that data. */
typedef struct strpool
{
  char *strs; /* table of strings (including terminators) */
  int   used;
}
strpool;

struct dict_t
{
  size_t   log2locpoolsz; /* log2 number of locations per locpool */
  size_t   log2strpoolsz; /* log2 number of bytes per strpool */

  locpool *locpools;      /* growable array of location pools */
  int      l_used;
  int      l_allocated;

  strpool *strpools;      /* growable array of string pools */
  int      s_used;
  int      s_allocated;
};

error dict__add_with_len(dict_t *d, const char *string, size_t len,
                         dict_index *index);

/* dict_index but returning the length of 'string' (from strlen) */
dict_index dict__index_and_len(dict_t *d, const char *string, size_t *len);

/* dict_index but taking the length of 'string' */
dict_index dict__index_with_len(dict_t *d, const char *string, size_t len);

error dict__ensure_loc_space(dict_t *d);
error dict__ensure_str_space(dict_t *d, size_t len);

#endif /* DICT_IMPL_H */
