/* --------------------------------------------------------------------------
 *    Name: clear.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

void vector_clear(vector_t *v)
{
  v->used = 0;
}
