/* --------------------------------------------------------------------------
 *    Name: count.c
 * Purpose: Bit array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/bitarr.h"

int bitarr_count(const struct bitarr_t *arr, size_t bytelen)
{
  const bitarr_elem_t *base;
  const bitarr_elem_t *end;
  int                  c;

  base = arr->entries;
  end  = base + (bytelen >> (BITARR_SHIFT - 3));

  c = 0;
  while (base != end)
    c += countbits(*base++);

  return c;
}
