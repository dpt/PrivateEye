/* --------------------------------------------------------------------------
 *    Name: set.c
 * Purpose: Bit vectors
 * Version: $Id: set.c,v 1.3 2009-05-18 22:07:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/errors.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

error bitvec__set(bitvec_t *v, int bit)
{
  error err;
  int   word;

  word = bit >> 5;

  err = bitvec__ensure(v, word + 1);
  if (err)
    return err;

  v->vec[word] |= 1u << (bit & 0x1F);

  return error_OK;
}
