/* --------------------------------------------------------------------------
 *    Name: drag-object.c
 * Purpose:
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/draganobject.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/vdu/screen.h"

#include "appengine/wimp/icon.h"

/* ----------------------------------------------------------------------- */

static int cmos;

/* ----------------------------------------------------------------------- */

void drag_object_box(wimp_w                w,
                     const os_box         *box,
                     int                   x,
                     int                   y,
                     drag_object_renderer *render,
                     void                 *opaque)
{
  wimp_window_state wstate;
  wimp_drag         drag;
  int               screen_x0, screen_y0;
  int               x0, y0, x1, y1;

  wstate.w = w;
  wimp_get_window_state(&wstate);

  screen_x0 = wstate.visible.x0 - wstate.xscroll;
  screen_y0 = wstate.visible.y1 - wstate.yscroll;

  x0 = screen_x0 + box->x0;
  y0 = screen_y0 + box->y0;
  x1 = screen_x0 + box->x1;
  y1 = screen_y0 + box->y1;

  drag.initial.x0 = x0;
  drag.initial.y0 = y0;
  drag.initial.x1 = x1;
  drag.initial.y1 = y1;

  cmos = osbyte2(osbyte_READ_CMOS,
                 osbyte_CONFIGURE_DRAG_ASPRITE,
                 0);
  if (cmos & osbyte_CONFIGURE_DRAG_ASPRITE_MASK)
  {
    bits              flags;
    os_register_block regs;

    flags = draganobject_HPOS_CENTRE   |
            draganobject_VPOS_CENTRE   |
            draganobject_BOUND_POINTER |
            draganobject_DROP_SHADOW   |
            draganobject_NO_DITHER     |
            draganobject_CALL_FUNCTION |
            draganobject_FUNCTION_USE_USER;

    regs.registers[0] = (int) opaque;

    /* enabled */
    draganobject_start(flags,
         (asm_routine) render,
                      &regs,
                      &drag.initial,
                       NULL);
  }
  else
  {
    /* disabled */

    drag.w = w;
    drag.type = wimp_DRAG_USER_FIXED;

    read_drag_box_for_screen(&drag.bbox);

    drag.bbox.x0 += x0 - x;
    drag.bbox.y0 += y0 - y;
    drag.bbox.x1 += x1 - x;
    drag.bbox.y1 += y1 - y;

    wimp_drag_box(&drag);
  }
}

void drag_object(wimp_w                w,
                 wimp_i                i,
                 int                   x,
                 int                   y,
                 drag_object_renderer *render,
                 void                 *opaque)
{
  wimp_icon_state istate;

  istate.w = w;
  istate.i = i;
  wimp_get_icon_state(&istate);

  drag_object_box(w, &istate.icon.extent, x, y, render, opaque);
}

void drag_object_stop(void)
{
  if (cmos & osbyte_CONFIGURE_DRAG_ASPRITE_MASK)
    draganobject_stop();
}
