/* --------------------------------------------------------------------------
 *    Name: string.c
 * Purpose: Dictionary
 * Version: $Id: string.c,v 1.4 2009-05-24 23:39:32 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "appengine/datastruct/dict.h"

const char *dict__string(dict_t *d, dict_index index)
{
  assert(d);

  return dict__string_and_len(d, NULL, index);
}
