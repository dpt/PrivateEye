/* --------------------------------------------------------------------------
 *    Name: mkgrey.c
 * Purpose: Populates a grey palette
 * Version: $Id: mkgrey.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/vdu/sprite.h"

void make_grey_palette(int log2bpp, unsigned int *palette)
{
  unsigned int step;
  unsigned int entry;

  switch (log2bpp)
  {
  case 0: step = 0xffffff00; break;
  case 1: step = 0x55555500; break;
  case 2: step = 0x11111100; break;
  case 3: step = 0x01010100; break;
  default: return;
  }

  entry = 0x00000000;
  do
  {
    *palette++ = entry;
    *palette++ = entry;
    entry += step;
  }
  while (entry != 0xffffff00 + step);
}
