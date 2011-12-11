/* --------------------------------------------------------------------------
 *    Name: width.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

int vector_width(const vector_t *v)
{
  return v->width;
}
