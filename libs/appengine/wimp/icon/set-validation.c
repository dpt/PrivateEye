
#include <string.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"
#include "appengine/base/strings.h"

void icon_set_validation(wimp_w w, wimp_i i, const char *validation)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if ((state.icon.flags & (wimp_ICON_TEXT | wimp_ICON_INDIRECTED)) == 0)
    return; /* not text + indirected */

  if (strcmp(state.icon.data.indirected_text.validation, validation) == 0)
  {
    return; /* exactly the same contents */
  }

  str_cpy(state.icon.data.indirected_text.validation, validation);

  /* Force a redraw of the icon */
  wimp_set_icon_state(w, i, 0, 0);
}
