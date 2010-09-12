/* --------------------------------------------------------------------------
 *    Name: get.c
 * Purpose: Bit vectors
 * Version: $Id: get.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

int bitvec__get(const bitvec_t *v, int bit)
{
  int word;

  word = bit >> 5;

  if (word >= v->length)
    return 0;

  return (v->vec[word] & (1u << (bit & 0x1F))) != 0;
}
