/* --------------------------------------------------------------------------
 *    Name: get.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

void *vector_get(vector_t *v, int i)
{
  return (char *) v->base + i * v->width;
}
