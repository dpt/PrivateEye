/* $Id: get-bbox.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include <string.h>

#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_get_bbox(wimp_w w, wimp_i i, os_box *bbox)
{
  wimp_icon_state state;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  memcpy(bbox, &state.icon.extent, sizeof(state.icon.extent));
}
