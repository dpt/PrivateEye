/* --------------------------------------------------------------------------
 *    Name: mask-data.c
 * Purpose: Returns a pointer to the sprite mask data
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

void *sprite_mask_data(const osspriteop_header *header)
{
  return (char *) header + header->mask;
}
