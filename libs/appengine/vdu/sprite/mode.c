/* --------------------------------------------------------------------------
 *    Name: mode.c
 * Purpose: Returns the best mode for the specified args
 * ----------------------------------------------------------------------- */

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

os_mode sprite_mode(int xeig, int yeig, int log2bpp)
{
  static const unsigned char tab[4][2][2] =
  {
    {
      { 25, 255, },
      {  0, 255, },
    },
    {
      { 26, 255, },
      {  8, 1,   },
    },
    {
      { 27, 255, },
      { 12,  9,  },
    },
    {
      { 28, 255, },
      { 15, 13,  },
    },
  };

  unsigned int mode;

  if (xeig >= 1 && yeig >= 1 && log2bpp <= 3)
  {
    mode = tab[log2bpp][yeig - 1][xeig - 1]; /* use table */
    if (mode != 255) /* 255 means 'no mode' */
      return (os_mode) mode;
  }

  /* 0 -> 180, 1 -> 90, 2 -> 45 */

  mode = osspriteop_NEW_STYLE;
  mode |= (180 >> xeig) << osspriteop_XRES_SHIFT;
  mode |= (180 >> yeig) << osspriteop_YRES_SHIFT;
  mode |= (1 + log2bpp) << osspriteop_TYPE_SHIFT;

  return (os_mode) mode;
}
