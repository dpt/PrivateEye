
#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_range_set_flags(wimp_w          w,
                          wimp_i          i_low,
                          wimp_i          i_high,
                          wimp_icon_flags eor,
                          wimp_icon_flags clear)
{
  wimp_icon_state state;
  wimp_i          i;

  state.w = w;
  for (i = i_low; i <= i_high; i++)
  {
    state.i = i;
    wimp_get_icon_state(&state);

    if (((state.icon.flags & ~clear) ^ eor) != state.icon.flags)
      wimp_set_icon_state(w, i, eor, clear);
  }
}
