/* --------------------------------------------------------------------------
 *    Name: rename.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <string.h>

#include "appengine/datastruct/dict.h"

#include "impl.h"

error dict__rename(dict_t *d, dict_index idx, const char *string)
{
  error       err;
  dict_index  newidx;
  loc        *p, *q;
  loc         t;

  assert(d);
  assert(string);

  if (!VALID(idx))
    return error_DICT_OUT_OF_RANGE;

  err = dict__add(d, string, &newidx);
  if (err == error_DICT_NAME_EXISTS && newidx == idx)
    /* renaming an index to its existing name is just ignored */
    return error_OK;
  else if (err)
    return err;

#ifndef NDEBUG
  /* scribble over the deleted copy */
  memset(PTR(idx), 'x', LENGTH(idx) - 1);
#endif

  dict__delete_index(d, idx);

  /* now transpose idx and newidx */

  p = &INDEXTOLOC(idx);
  q = &INDEXTOLOC(newidx);

  t = *p;
  *p = *q;
  *q = t;

  return error_OK;
}
