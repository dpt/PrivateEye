
#include <stdlib.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

double icon_get_double(wimp_w w, wimp_i i)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if (state.icon.flags & wimp_ICON_INDIRECTED)
    return atof(state.icon.data.indirected_text.text);
  else
    return 0.0;
}
