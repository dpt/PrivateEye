/* --------------------------------------------------------------------------
 *    Name: data.c
 * Purpose: Returns a pointer to the sprite data
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

void *sprite_data(const osspriteop_header *header)
{
  return (char *) header + header->image;
}
