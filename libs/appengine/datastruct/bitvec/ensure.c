/* --------------------------------------------------------------------------
 *    Name: ensure.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

error bitvec__ensure(bitvec_t *v, int need)
{
  if (need > v->length)
  {
    int           length;
    unsigned int *vec;

    length = need;

    vec = realloc(v->vec, length * sizeof(*v->vec));
    if (vec == NULL)
      return error_OOM;

    /* wipe the new segment */
    memset(vec + v->length, 0, (length - v->length) * sizeof(*v->vec));

    v->length = length;
    v->vec    = vec;
  }

  return error_OK;
}
