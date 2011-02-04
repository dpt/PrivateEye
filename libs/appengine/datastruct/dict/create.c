/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Dictionary
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/dict.h"

#include "impl.h"

dict_t *dict__create(void)
{
  return calloc(1, sizeof(dict_t));
}
