/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Dictionary
 * Version: $Id: create.c,v 1.2 2008-08-05 22:04:51 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/datastruct/dict.h"

#include "impl.h"

dict_t *dict__create(void)
{
  return calloc(1, sizeof(dict_t));
}
