/* --------------------------------------------------------------------------
 *    Name: toggle.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include "appengine/base/errors.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

error bitvec__toggle(bitvec_t *v, int bit)
{
  error err;
  int   word;

  word = bit >> 5;

  err = bitvec__ensure(v, word + 1);
  if (err)
    return err;

  v->vec[word] ^= 1u << (bit & 0x1F);

  return error_OK;
}
