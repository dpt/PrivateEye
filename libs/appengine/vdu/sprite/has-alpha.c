/* --------------------------------------------------------------------------
 *    Name: has-alpha.c
 * Purpose: Returns whether a sprite has alpha data
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

osbool sprite_has_alpha(const osspriteop_header *header)
{
  unsigned int *p;
  int           y,x;

  if (((osspriteop_mode_word) header->mode >> osspriteop_TYPE_SHIFT) != osspriteop_TYPE32BPP)
    return FALSE;

  p = sprite_data(header);

  /* no wastage in 32bpp sprites */

  /* 255..0 is opaque..transparent for Tinct/Variations/etc., but sprites are
   * created by default with 0 in their spare byte, so all-zero means "no
   * alpha" as does all-255. */

  int lowest, highest;

  lowest  = 256;
  highest = -1;
  for (y = 0; y < header->height + 1; y++)
    for (x = 0; x < header->width + 1; x++)
    {
      int a;

      a = (*p++ & 0xff000000) >> 24;
      if (a >= 1 && a <= 254)
        return TRUE; /* must have alpha */
      if (a < lowest)
        lowest = a;
      if (a > highest)
        highest = a;
      if (lowest != highest)
        return TRUE; /* return true as soon as we see differing alphas */
    }

  return FALSE;
}
