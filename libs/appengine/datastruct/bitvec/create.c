/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Bit vectors
 * Version: $Id: create.c,v 1.4 2010-01-06 00:36:20 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

bitvec_t *bitvec__create(int length)
{
  error     err;
  bitvec_t *v;

  v = malloc(sizeof(*v));
  if (v == NULL)
    return NULL;

  v->length = 0;
  v->vec    = NULL;

  err = bitvec__ensure(v, length >> 5);
  if (err)
  {
    assert(err == error_OOM);
    return NULL;
  }

  return v;
}
