/* read-icon-bar.c */

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

int read_icon_bar_height(void)
{
  static int cache;

  if (cache == 0)
  {
    wimp_window_state state;

    state.w = wimp_ICON_BAR;
    wimp_get_window_state(&state);

    cache = state.visible.y1;
  }
  return cache;
}

int read_icon_bar_unobscured(void)
{
  /* This overlap is correct for EY1 modes, but I can't really detect a
   * pattern in the values that the Wimp is using for other shaped modes.
   */
  const int overlap = 12; /* OS units */

  return read_icon_bar_height() - overlap;
}
