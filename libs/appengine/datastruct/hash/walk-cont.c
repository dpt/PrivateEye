/* --------------------------------------------------------------------------
 *    Name: walk-cont.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/primes.h"

#include "appengine/datastruct/hash.h"

#include "impl.h"

int hash_walk_continuation(hash_t      *h,
                            int          continuation,
                            const void **key,
                            const void **value)
{
  unsigned int bin;
  unsigned int item;
  int          i;
  node        *n;
  node        *next;

  /* The continuation value is treated as a pair of 16-bit fields, the top
   * half being the bin and the bottom half being the node within the bin. */

  bin  = ((unsigned int) continuation & 0xffff0000) >> 16;
  item = ((unsigned int) continuation & 0x0000ffff) >> 0;

  if (bin >= h->nbins)
    return -1; /* invalid continuation value */

  /* if we're starting off, scan forward to the first occupied bin */

  if (bin == 0)
    while (h->bins[bin] == NULL)
      bin++;

  i = 0; /* node counter */

  for (n = h->bins[bin]; n; n = next)
  {
    next = n->next;

    if (i == item)
      break;

    i++;
  }

  if (n == NULL) /* invalid continuation value */
    return -1;

  *key   = n->key;
  *value = n->value;

  /* form the continuation value and return it */

  /* a chain this long would be very odd, but ... */
  assert(i + 1 <= 0xffff);

  if (next)
    return (bin << 16) | (i + 1); /* current bin, next node */

  /* scan forward to the next occupied bin */

  while (++bin < h->nbins)
    if (h->bins[bin])
      return bin << 16; /* next occupied bin, first node */

  return 0; /* ran out of bins */
}
