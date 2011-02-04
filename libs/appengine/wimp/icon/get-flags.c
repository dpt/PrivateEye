
#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

wimp_icon_flags icon_get_flags(wimp_w w, wimp_i i)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  return state.icon.flags;
}
