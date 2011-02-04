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
  if (d)
  {
    free(d->data);
    free(d->strings);
    free(d);
  }
}
