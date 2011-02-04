/* --------------------------------------------------------------------------
 *    Name: size.c
 * Purpose:
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void size_icon(wimp_w w, wimp_i i, int width, int height)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  wimp_resize_icon(w, i, state.icon.extent.x0,
                         state.icon.extent.y1 - height,
                         state.icon.extent.x0 + width,
                         state.icon.extent.y1);
}
