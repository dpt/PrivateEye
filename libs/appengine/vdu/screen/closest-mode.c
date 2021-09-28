/* closest-mode.c */

#include <limits.h>
#include <stdio.h>

#include "oslib/os.h"

os_mode closest_mode(int min_width, int min_height, int pref_log2bpp)
{
  static const os_mode_flags useful_mode_flags = os_MODE_FLAG_NON_GRAPHICS |
                                                 os_MODE_FLAG_TELETEXT     |
                                                 os_MODE_FLAG_GAP          |
                                                 os_MODE_FLAG_BBC_GAP;

  const int    total_pixels = min_width * min_height;

  int          mode;
  unsigned int bestscore = UINT_MAX;
  int          bestmode  = -1;

  for (mode = 0; mode <= 127; mode++)
  {
    int          mode_status;
    bits         psr;
    int          modeflags;
    int          width, height, log2bpp;
    int          wasted_pixels;
    int          log2bpp_delta;
    unsigned int score;

    /* reject unselectable modes */
    psr = os_check_mode_valid((os_mode) mode, &mode_status, NULL);
    if (psr & (2u << 28))
      continue;

    /* reject invalid modes */
    psr = os_read_mode_variable((os_mode) mode, os_MODEVAR_MODE_FLAGS, &modeflags);
    if (psr & (2u << 28))
      continue;

    /* reject non-graphics modes */
    if ((modeflags & useful_mode_flags) != 0)
      continue;

    os_read_mode_variable((os_mode) mode, os_MODEVAR_XWIND_LIMIT, &width);
    os_read_mode_variable((os_mode) mode, os_MODEVAR_YWIND_LIMIT, &height);
    os_read_mode_variable((os_mode) mode, os_MODEVAR_LOG2_BPP,    &log2bpp);

    width  += 1;
    height += 1;

    /* reject too-small modes */
    if (width < min_width || height < min_height)
      continue;

    wasted_pixels = (width * height) - total_pixels;

    log2bpp_delta = log2bpp - pref_log2bpp;
    if (log2bpp_delta < 0)
      log2bpp_delta = -log2bpp_delta;

    score = (wasted_pixels << 8) + log2bpp_delta; /* lower scores are better */
    if (score < bestscore)
    {
      bestscore = score;
      bestmode  = mode;

      if (score == 0)
        break; /* best possible match */
    }
  }

  return (os_mode) bestmode;
}

