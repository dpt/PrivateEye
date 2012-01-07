/* --------------------------------------------------------------------------
 *    Name: walk.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/primes.h"

#include "appengine/datastruct/hash.h"

#include "impl.h"

int hash__walk(hash_t *h, hash__walk_callback *cb, void *cbarg)
{
  int i;

  for (i = 0; i < h->nbins; i++)
  {
    node *n;
    node *next;

    for (n = h->bins[i]; n != NULL; n = next)
    {
      int r;

      next = n->next;

      r = cb(n->key, n->value, cbarg);
      if (r < 0)
        return -1;
    }
  }

  return 0;
}
