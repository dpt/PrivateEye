/* --------------------------------------------------------------------------
 *    Name: length.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

int bitvec_length(const bitvec_t *v)
{
  return v->length << 5;
}
