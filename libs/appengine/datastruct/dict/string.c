/* --------------------------------------------------------------------------
 *    Name: string.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "appengine/datastruct/dict.h"

const char *dict__string(dict_t *d, dict_index index)
{
  assert(d);

  return dict__string_and_len(d, NULL, index);
}
