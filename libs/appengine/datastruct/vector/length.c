/* --------------------------------------------------------------------------
 *    Name: length.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

int vector_length(const vector_t *v)
{
  return v->used;
}
