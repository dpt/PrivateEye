/* --------------------------------------------------------------------------
 *    Name: set-all.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include <string.h>

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

void bitvec_set_all(bitvec_t *v)
{
  // this is probably doing the wrong thing by setting all 'known' bits.
  // what you'd expect is that every single bit (including those unallocated)
  // would now be returned as one, so we'd need a flag to say what to read
  // unallocated bits as

  /* Note: This may set more bits than strictly required. */

  memset(v->vec, 0xff, v->length * 4);
}
