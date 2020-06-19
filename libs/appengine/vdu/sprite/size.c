/* --------------------------------------------------------------------------
 *    Name: size.c
 * Purpose: Calculates the byte size of a sprite
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/types.h"
#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

size_t sprite_size(int w, int h, int log2bpp, osbool paletted)
{
  size_t headers, palette, rowbytes;

  headers  = sizeof(osspriteop_area) + sizeof(osspriteop_header);
  palette  = paletted ? (8 << log2bpp) : 0;
  rowbytes = (((w << log2bpp) + 31) & ~31) >> 3;

  return headers + palette + rowbytes * h;
}
