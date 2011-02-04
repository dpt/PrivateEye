/* --------------------------------------------------------------------------
 *    Name: drag-icon.c
 * Purpose:
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/dragasprite.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/vdu/screen.h"
#include "appengine/wimp/window.h"

#include "appengine/wimp/icon.h"

/* ----------------------------------------------------------------------- */

static int cmos;

/* ----------------------------------------------------------------------- */

void drag_icon(wimp_w w, wimp_i i, int x, int y, const char *sprite)
{
  wimp_window_state wstate;
  wimp_icon_state   istate;
  wimp_drag         drag;
  int               screen_x0, screen_y0;
  int               x0, y0, x1, y1;

  wstate.w = w;
  wimp_get_window_state(&wstate);

  screen_x0 = wstate.visible.x0 - wstate.xscroll;
  screen_y0 = wstate.visible.y1 - wstate.yscroll;

  istate.w = w;
  istate.i = i;
  wimp_get_icon_state(&istate);

  x0 = screen_x0 + istate.icon.extent.x0;
  y0 = screen_y0 + istate.icon.extent.y0;
  x1 = screen_x0 + istate.icon.extent.x1;
  y1 = screen_y0 + istate.icon.extent.y1;

  drag.w = w;
  drag.type = wimp_DRAG_USER_FIXED;
  drag.initial.x0 = x0;
  drag.initial.y0 = y0;
  drag.initial.x1 = x1;
  drag.initial.y1 = y1;

  read_drag_box_for_screen(&drag.bbox);

  drag.bbox.x0 += x0 - x;
  drag.bbox.y0 += y0 - y;
  drag.bbox.x1 += x1 - x;
  drag.bbox.y1 += y1 - y;

  cmos = osbyte2(osbyte_READ_CMOS,
                 osbyte_CONFIGURE_DRAG_ASPRITE,
                 0);
  if (cmos & osbyte_CONFIGURE_DRAG_ASPRITE_MASK)
  {
    os_error        *error;
    osspriteop_area *area;

    /* enabled */

    /* is it one of our sprites? */

    area = window_get_sprite_area();

    error = xosspriteop_read_sprite_info(osspriteop_NAME,
                                         area,
                         (osspriteop_id) sprite,
                                         NULL, NULL, NULL, NULL);

    if (error)
      area = dragasprite_WIMP_SPRITE_AREA;

    dragasprite_start(dragasprite_HPOS_CENTRE | dragasprite_VPOS_CENTRE | dragasprite_BOUND_POINTER | dragasprite_DROP_SHADOW,
                      area,
                      sprite,
                     &drag.initial,
                     &drag.bbox);
  }
  else
  {
    /* disabled */
    wimp_drag_box(&drag);
  }
}

void drag_icon_stop(void)
{
  if (cmos & osbyte_CONFIGURE_DRAG_ASPRITE_MASK)
    dragasprite_stop();
}
