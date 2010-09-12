/* --------------------------------------------------------------------------
 *    Name: clear.c
 * Purpose: Bit vectors
 * Version: $Id: clear.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

void bitvec__clear(bitvec_t *v, int bit)
{
  int word;

  word = bit >> 5;

  if (word >= v->length)
    return;

  v->vec[word] &= ~(1u << (bit & 0x1F));
}
