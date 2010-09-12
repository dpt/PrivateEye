/* --------------------------------------------------------------------------
 *    Name: data.c
 * Purpose: Returns a pointer to the sprite data
 * Version: $Id: data.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

void *sprite_data(const osspriteop_header *header)
{
  return (char *) header + header->image;
}
