/* --------------------------------------------------------------------------
 *    Name: clear-all.c
 * Purpose: Bit vectors
 * Version: $Id: clear-all.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <string.h>

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

void bitvec__clear_all(bitvec_t *v)
{
  /* Note: This may clear more bits than strictly required. */

  memset(v->vec, 0x00, v->length * 4);
}
