/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

bitvec_t *bitvec_create(int length)
{
  error     err;
  bitvec_t *v;

  v = malloc(sizeof(*v));
  if (v == NULL)
    return NULL;

  v->length = 0;
  v->vec    = NULL;

  err = bitvec_ensure(v, length >> 5);
  if (err)
  {
    assert(err == error_OOM);
    return NULL;
  }

  return v;
}
