/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

vector_t *vector_create(size_t width)
{
  vector_t *v;

  v = calloc(1, sizeof(*v));
  if (v == NULL)
    return NULL;

  v->width = width;

  return v;
}
