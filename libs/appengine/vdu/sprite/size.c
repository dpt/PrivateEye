/* --------------------------------------------------------------------------
 *    Name: size.c
 * Purpose: Calculates the byte size of a sprite
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

int sprite_size(int w, int h, int log2bpp)
{
  int rowbytes;

  rowbytes = (((w << log2bpp) + 31) & ~31) >> 3;

  return sizeof(osspriteop_area) + sizeof(osspriteop_header) + rowbytes * h;
}
