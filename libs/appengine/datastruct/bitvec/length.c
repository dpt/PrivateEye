/* --------------------------------------------------------------------------
 *    Name: length.c
 * Purpose: Bit vectors
 * Version: $Id: length.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

int bitvec__length(const bitvec_t *v)
{
  return v->length << 5;
}
