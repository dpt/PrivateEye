/* --------------------------------------------------------------------------
 *    Name: bsearch.h
 * Purpose: Searching arrays
 * Version: $Id: bsearch-impl.h,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#if !defined(TYPE) || !defined(NAME)
#error TYPE or NAME must be defined.
#endif

#include <assert.h>
#include <stddef.h>

#include "appengine/base/bitwise.h"

#include "appengine/base/bsearch.h"

int NAME(const TYPE *array, size_t nelems, size_t stride, TYPE want)
{
  size_t searchRange;
  size_t i;
  TYPE   c;

  assert(nelems == 0 || array != NULL);
  assert(stride > 0);

  if (nelems == 0)
    return -1;

  stride /= sizeof(*array);

  searchRange = power2le(nelems);

  i = searchRange - 1;
  if (want > array[i * stride])
    i = nelems - searchRange; /* rangeShift */

  do
  {
    searchRange >>= 1;

    c = array[i * stride];

    if (want < c)
      i -= searchRange;
    else if (want > c)
      i += searchRange;
    else
      break;
  }
  while (searchRange != 0);

  if (want == c)
  {
    assert(i < nelems);
    return i;
  }

  return -1;
}
