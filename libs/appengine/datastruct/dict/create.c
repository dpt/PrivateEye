/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/dict.h"

#include "impl.h"

dict_t *dict__create(void)
{
  return dict__create_tuned(0, 0);
}

dict_t *dict__create_tuned(size_t locpoolsz, size_t strpoolsz)
{
  dict_t *d;

  d = calloc(1, sizeof(dict_t));
  if (d == NULL)
    return NULL;

  d->log2locpoolsz = locpoolsz ? ceillog2(locpoolsz) : LOG2LOCPOOLSZ;
  d->log2strpoolsz = strpoolsz ? ceillog2(strpoolsz) : LOG2STRPOOLSZ;

  return d;
}
