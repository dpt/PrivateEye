/* $Id: get-text.c,v 1.2 2009-05-18 22:07:52 dpt Exp $ */

#include <string.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"
#include "appengine/base/strings.h"

void icon_get_text(wimp_w w, wimp_i i, char *buffer)
{
  wimp_icon_state state;
  const char     *text;
  int             size;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  if (state.icon.flags & wimp_ICON_INDIRECTED)
  {
    text = state.icon.data.indirected_text.text;
    size = state.icon.data.indirected_text.size - 1;
  }
  else
  {
    text = state.icon.data.text;
    size = 11;
  }

  str_n_cpy(buffer, text, size);
}
