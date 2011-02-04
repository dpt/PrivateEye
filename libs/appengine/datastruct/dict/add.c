/* --------------------------------------------------------------------------
 *    Name: add.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

// TODO
//
// might want to include deleted elements in this search. the strings
// will still be in the array (but the length with be zapped at this
// point.)

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/dict.h"

#include "impl.h"

enum
{
  StringMinimum = 64, /* characters */
  DataMinimum   = 8,  /* entries    */
};

/* ----------------------------------------------------------------------- */

error dict__ensure_string_space(dict_t *d, size_t len)
{
  assert(d);

  if (d->s_allocated - d->s_used < len)
  {
    /* need more space */
    size_t newallocated;
    void  *newstrings;

    newallocated = d->s_used + len;
    /* start with at least this many chars */
    if (newallocated < StringMinimum)
      newallocated = StringMinimum;
    /* subtract 1 to make it greater than or equal */
    newallocated = (size_t) power2gt(newallocated - 1);

    newstrings = realloc(d->strings, newallocated * sizeof(*d->strings));
    if (newstrings == NULL)
      return error_OOM;

    d->strings     = newstrings;
    d->s_allocated = newallocated;
  }

  return error_OK;
}

error dict__ensure_data_space(dict_t *d, size_t n)
{
  assert(d);

  if (d->d_allocated - d->d_used < n)
  {
    /* need more space */
    size_t newallocated;
    void  *newdata;

    newallocated = d->d_used + n;
    /* start with at least this many entries */
    if (newallocated < DataMinimum)
      newallocated = DataMinimum;
    /* subtract 1 to make it greater than or equal */
    newallocated = (size_t) power2gt(newallocated - 1);

    newdata = realloc(d->data, newallocated * sizeof(*d->data));
    if (newdata == NULL)
      return error_OOM;

    d->data        = newdata;
    d->d_allocated = newallocated;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error dict__add(dict_t *d, const char *string, dict_index *index)
{
  assert(d);
  assert(string);
  assert(index);

  return dict__add_with_len(d, string, strlen(string), index);
}

error dict__add_with_len(dict_t *d, const char *string, size_t len,
                         dict_index *index)
{
  error       err;
  dict_index  i;
  char       *ds;

  assert(d);
  assert(string);

  i = dict__index_with_len(d, string, len);
  if (i >= 0) /* already present */
  {
    if (index)
      *index = i;
    return error_DICT_NAME_EXISTS;
  }

  len++; /* for terminator */

  err = dict__ensure_string_space(d, len);
  if (err)
    return err;

  err = dict__ensure_data_space(d, 1);
  if (err)
    return err;

  /* don't assume there's a terminator following 'string' */

  ds = d->strings + d->s_used;
  memcpy(ds, string, len - 1);
  ds[len - 1] = '\0';
  d->s_used += len;

  i = d->d_used++;
  d->data[i].offset = d->s_used - len;
  d->data[i].length = len;

  if (index)
    *index = i;

  return error_OK;
}
