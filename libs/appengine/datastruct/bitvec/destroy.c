/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Bit vectors
 * Version: $Id: destroy.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
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
