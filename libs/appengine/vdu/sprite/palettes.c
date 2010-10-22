/* --------------------------------------------------------------------------
 *    Name: palettes.c
 * Purpose: Default desktop palette definitions
 * Version: $Id: palettes.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"

#include "appengine/vdu/sprite.h"

static const os_PALETTE(2) default_1bpp =
{
  { 0xffffff00, 0x00000000 }
};

static const os_PALETTE(4) default_2bpp =
{
  { 0xffffff00, 0xbbbbbb00, 0x77777700, 0x00000000 }
};

static const os_PALETTE(16) default_4bpp =
{
  { 0xffffff00, 0xdddddd00, 0xbbbbbb00, 0x99999900,
    0x77777700, 0x55555500, 0x33333300, 0x00000000,
    0x99440000, 0x00eeee00, 0x00cc0000, 0x0000dd00,
    0xbbeeee00, 0x00885500, 0x00bbff00, 0xffbb0000 }
};

static os_palette *default256(void)
{
  static os_PALETTE(256) *palette = NULL;

  int i;
  os_colour *p;

  if (palette)
    return (os_palette *) palette;

  palette = malloc(os_SIZEOF_PALETTE(256));
  if (palette == NULL)
    return NULL;

  p = palette->entries;

  for (i = 0; i < 256; i++)
  {
    os_colour e;

    e = ((i & 0x80) << 24) |
        ((i & 0x08) << 27) |
        ((i & 0x03) << 28) |
        ((i & 0x60) << 17) |
        ((i & 0x03) << 20) |
        ((i & 0x10) << 11) |
        ((i & 0x07) << 12);

    *p++ = e | (e >> 4);
  }

  return (os_palette *) palette;
}

const os_palette *get_default_palette(int log2bpp)
{
  switch (log2bpp)
  {
  case 0: return (const os_palette *) &default_1bpp;
  case 1: return (const os_palette *) &default_2bpp;
  case 2: return (const os_palette *) &default_4bpp;
  case 3: return default256();
  default: return NULL;
  }
}
