/* --------------------------------------------------------------------------
 *    Name: clear.c
 * Purpose: Clear to colour effect
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/os.h"
#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

#include "appengine/graphics/sprite-effects.h"

/* Takes os_colour, returns grey pixel byte value. */
static unsigned char colour_to_grey(os_colour colour)
{
  const int red_weight   = 19595; /* 0.29900 * 65536 (rounded down) */
  const int green_weight = 38470; /* 0.58700 * 65536 (rounded up)   */
  const int blue_weight  =  7471; /* 0.11400 * 65536 (rounded down) */

  int r, g, b;
  int gr;

  colour >>= 8; /* 0xBBGGRRXX -> 0x00BBGGRR */

  r = (colour >> 0)  & 0xff;
  g = (colour >> 8)  & 0xff;
  b = (colour >> 16) & 0xff;

  gr = (r * red_weight + g * green_weight + b * blue_weight) >> 16;

  return (gr << 0 ) | (gr << 8 ) | (gr << 16);
}

static void clear_8(osspriteop_header *src,
                    osspriteop_header *dst,
                    int                width,
                    int                height,
                    os_colour          colour)
{
  unsigned char *dp;

  NOT_USED(src);

  colour = colour_to_grey(colour);

  dp = sprite_data(dst);

  // shouldn't the width be rounded up to include padding here?
  memset(dp, colour, width * height);
}

static void clear_888(osspriteop_header *src,
                      osspriteop_header *dst,
                      int                width,
                      int                height,
                      os_colour          colour)
{
  int           xy;
  unsigned int *sp;
  unsigned int *dp;

  colour >>= 8; /* 0xBBGGRRXX -> 0x00BBGGRR */

  sp = sprite_data(src);
  dp = sprite_data(dst);

  for (xy = width * height; xy--; )
    *dp++ = colour | (*sp++ & 0xff000000);
}

result_t effects_clear_apply(osspriteop_area   *area,
                          osspriteop_header *src,
                          osspriteop_header *dst,
                          os_colour          colour)
{
  int                  width, height;
  osspriteop_mode_word mode;
  int                  log2bpp;

  sprite_info(area, src, &width, &height, NULL, (os_mode *) &mode, &log2bpp);

  switch (log2bpp)
  {
  case 3:
    clear_8(src, dst, width, height, colour);
    break;

  case 5:
    clear_888(src, dst, width, height, colour);
    break;

  default:
    return result_SPRITEFX_UNSUPP_EFFECT;
  }

  return result_OK;
}
