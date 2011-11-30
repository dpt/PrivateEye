/* --------------------------------------------------------------------------
 *    Name: delete.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include "appengine/datastruct/dict.h"

void dict__delete(dict_t *d, const char *string)
{
  dict__delete_index(d, dict__index(d, string));
}
