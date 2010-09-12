/* --------------------------------------------------------------------------
 *    Name: or.c
 * Purpose: Bit vectors
 * Version: $Id: or.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "oslib/macros.h"

#include "appengine/datastruct/bitvec.h"

#include "impl.h"

error bitvec__or(const bitvec_t *a, const bitvec_t *b, bitvec_t **c)
{
  int             min, max;
  bitvec_t       *v;
  int             i;
  const bitvec_t *p;

  *c = NULL;

  min = MIN(a->length, b->length);
  max = MAX(a->length, b->length);

  /* create vec, ensuring that enough space is allocated to perform the op */
  v = bitvec__create(max << 5);
  if (v == NULL)
    return error_OOM;

  for (i = 0; i < min; i++)
    v->vec[i] = a->vec[i] | b->vec[i];

  p = (a->length > b->length) ? a : b;

  for (i = min; i < max; i++)
    v->vec[i] = p->vec[i];

  *c = v;

  return error_OK;
}
