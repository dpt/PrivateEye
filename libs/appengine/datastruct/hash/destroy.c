/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Hash
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/datastruct/hash.h"

#include "impl.h"

void hash__destroy(hash_t *h)
{
  int i;

  for (i = 0; i < h->nbins; i++)
    while (h->bins[i])
      hash__remove_node(h, &h->bins[i]);

  free(h->bins);

  free(h);
}
