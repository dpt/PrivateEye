/* --------------------------------------------------------------------------
 *    Name: delete.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "oslib/types.h"

#include "appengine/datastruct/dict.h"

#include "impl.h"

void dict__delete(dict_t *d, const char *string)
{
  dict_index index;

  index = dict__index(d, string);
  if ((unsigned int) index >= d->d_used)
    return; /* out of range */

  d->data[index].length = -d->data[index].length;

  /* this leaves the data in place but inaccessible */
  /* certain add-delete-add cases could make the array grow unbounded */
}
