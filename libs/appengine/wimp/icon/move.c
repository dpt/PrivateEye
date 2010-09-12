/* --------------------------------------------------------------------------
 *    Name: move-icon.c
 * Purpose:
 * Version: $Id: move.c,v 1.1 2009-04-29 23:32:01 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void move_icon(wimp_w w, wimp_i i, int x, int y)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  wimp_resize_icon(w, i,
                   x, y,
                   x + (state.icon.extent.x1 - state.icon.extent.x0),
                   y + (state.icon.extent.y1 - state.icon.extent.y0));
}
