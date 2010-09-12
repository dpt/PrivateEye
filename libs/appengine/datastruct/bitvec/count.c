/* --------------------------------------------------------------------------
 *    Name: count.c
 * Purpose: Bit vectors
 * Version: $Id: count.c,v 1.3 2010-01-06 00:36:20 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/bitvec.h"

#include "impl.h"

int bitvec__count(const bitvec_t *v)
{
  int c;
  int i;

  c = 0;
  for (i = 0; i < v->length; i++)
    c += countbits(v->vec[i]);

  return c;
}
