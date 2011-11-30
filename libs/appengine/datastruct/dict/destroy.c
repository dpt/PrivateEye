/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/dict.h"

#include "impl.h"

void dict__destroy(dict_t *d)
{
  int i;

  if (d == NULL)
    return;

  for (i = 0; i < d->l_used; i++)
    free(d->locpools[i].locs);

  free(d->locpools);

  for (i = 0; i < d->s_used; i++)
    free(d->strpools[i].strs);

  free(d->strpools);

  free(d);
}
