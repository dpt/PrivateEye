/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

void bitvec__destroy(bitvec_t *v)
{
  free(v->vec);
  free(v);
}
