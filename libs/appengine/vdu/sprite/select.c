/* --------------------------------------------------------------------------
 *    Name: select.c
 * Purpose: Returns the N'th sprite in the area
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

osspriteop_header *sprite_select(const osspriteop_area *area, int n)
{
  osspriteop_header *h;

  if (n >= area->sprite_count)
    return NULL;

  h = (osspriteop_header *) ((char *) area + area->first);

  while (n--)
    h = (osspriteop_header *) ((char *) h + h->size);

  return h;
}
