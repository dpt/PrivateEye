
#include <stdlib.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

int icon_get_int(wimp_w w, wimp_i i)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if (state.icon.flags & wimp_ICON_INDIRECTED)
    return atoi(state.icon.data.indirected_text.text);
  else
    return 0;
}
