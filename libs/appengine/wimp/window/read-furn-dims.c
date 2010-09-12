/* --------------------------------------------------------------------------
 *    Name: read-furn-dims.c
 * Purpose: read_furniture_dimensions
 * Version: $Id: read-furn-dims.c,v 1.1 2009-04-29 23:32:01 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void read_furniture_dimensions(wimp_w w, os_box *furn)
{
  wimp_outline      outline;
  wimp_window_state state;
  os_box            f;

  outline.w = w;
  wimp_get_window_outline(&outline);

  state.w = w;
  wimp_get_window_state(&state);

  f.x0 = state.visible.x0 - outline.outline.x0;
  f.y0 = state.visible.y0 - outline.outline.y0;
  f.x1 = outline.outline.x1 - state.visible.x1;
  f.y1 = outline.outline.y1 - state.visible.y1;

  *furn = f;
}
