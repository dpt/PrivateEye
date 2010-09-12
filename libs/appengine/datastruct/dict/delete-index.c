/* --------------------------------------------------------------------------
 *    Name: delete-index.c
 * Purpose: Dictionary
 * Version: $Id: delete-index.c,v 1.4 2009-05-24 23:39:32 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "oslib/types.h"

#include "appengine/datastruct/dict.h"

#include "impl.h"

void dict__delete_index(dict_t *d, dict_index index)
{
  assert(d);

  if ((unsigned int) index >= d->d_used)
    return; /* out of range */

  d->data[index].length = -d->data[index].length;

  /* this leaves the data in place but inaccessible */
  /* certain add-delete-add cases could make the array grow unbounded */
}
