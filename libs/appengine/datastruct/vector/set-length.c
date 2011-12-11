/* --------------------------------------------------------------------------
 *    Name: set-length.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

error vector_set_length(vector_t *v, size_t length)
{
  void *newbase;

  newbase = realloc(v->base, length * v->width);
  if (newbase == NULL)
    return error_OOM;

  v->used      = length;
  v->allocated = length;
  v->base      = newbase;

  return error_OK;
}
