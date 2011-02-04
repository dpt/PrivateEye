
#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_group_set_flags(wimp_w w, const wimp_i *icons, int nicons,
                          wimp_icon_flags eor, wimp_icon_flags clear)
{
  wimp_icon_state state;
  int             i;

  state.w = w;

  for (i = 0; i < nicons; i++)
  {
    state.i = icons[i];

    wimp_get_icon_state(&state);

    if (((state.icon.flags & ~clear) ^ eor) != state.icon.flags)
      wimp_set_icon_state(w, icons[i], eor, clear);
  }
}
