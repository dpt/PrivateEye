/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Dictionary
 * Version: $Id: destroy.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
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
