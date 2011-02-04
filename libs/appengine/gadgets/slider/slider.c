/* --------------------------------------------------------------------------
 *    Name: slider.c
 * Purpose: Slider gadgets
 * ----------------------------------------------------------------------- */

#include "swis.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"

#include "appengine/gadgets/slider.h"

#define GAP 12

/* ----------------------------------------------------------------------- */

static struct
{
  wimp_w         w;
  wimp_i         i; /* foreground icon */
  int            x;
  int            y;
  os_box         bbox;
  slider_update *callback;
  int            val; /* last reported value */
  int            min, max;
}
slider;

/* ----------------------------------------------------------------------- */

static event_wimp_handler slider_event_user_drag_box,
                          slider_event_pollword_non_zero;

/* ----------------------------------------------------------------------- */

static void slider_set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX,     slider_event_user_drag_box     },
    { wimp_POLLWORD_NON_ZERO, slider_event_pollword_non_zero },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);
}

/* ----------------------------------------------------------------------- */

static int slider_event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  slider_set_handlers(0);

  return event_HANDLED;
}

static void slider_resize(wimp_pointer *pointer)
{
  int             s_range;
  int             s_width;
  int             newx;
  int             half;
  int             val;
  wimp_icon_state iconstate;
  int             oldx;

  // hoist these
  s_range = slider.max - slider.min;
  s_width = slider.bbox.x1 - slider.bbox.x0;

  /* make pointer work area relative, then make it relative to the slider
   * icon */
  newx = pointer->pos.x + slider.x - slider.bbox.x0;

  half = (s_width - 1) / 2;
  val  = (newx * s_range + half) / (s_width - 1);

  if (val + slider.min == slider.val)
    return; /* no change */

  /* check to see if we've got fewer output values than input values.
   * if so, round the output coord to that the slider 'steps'. */
  if (s_range < s_width) // tenuous?
    /* work out where newx really should lie */
    newx = val * (s_width - 1) / s_range;

  newx += slider.bbox.x0 + 1; /* relative to slider icon, also make excl. */

  iconstate.w = slider.w;
  iconstate.i = slider.i;
  wimp_get_icon_state(&iconstate); // avoid this?

  wimp_resize_icon(slider.w, slider.i,
                   slider.bbox.x0, slider.bbox.y0, newx, slider.bbox.y1);
  wimp_resize_icon(slider.w, slider.i - 1,
                   newx, slider.bbox.y0, slider.bbox.x1, slider.bbox.y1);

  oldx = iconstate.icon.extent.x1;

  if (newx < oldx)
  {
    int temp;

    temp = oldx;
    oldx = newx;
    newx = temp;
  }

  wimp_force_redraw(slider.w, oldx, slider.bbox.y0, newx, slider.bbox.y1);


  val += slider.min;
  slider.val = val;
  slider.callback(slider.i - 2, val); /* callback quoting the 'pit' icon */
}

static int slider_event_pollword_non_zero(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer pointer;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  event_zero_pollword();

  wimp_get_pointer_info(&pointer);

  slider_resize(&pointer);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void slider_start(wimp_pointer *pointer, slider_update *update,
                  int min, int max)
{
  wimp_window_state state;
  int               x,y;
  wimp_i            i;
  wimp_icon_state   iconstate;
  wimp_drag         drag;

  state.w = pointer->w;
  wimp_get_window_state(&state);

  /* work area pixels at origin */
  x = state.xscroll - state.visible.x0;
  y = state.yscroll - state.visible.y1;

  i = pointer->i;

  iconstate.w = pointer->w;
  iconstate.i = i;
  wimp_get_icon_state(&iconstate);

  /* detect which icon was poked */
  if (((iconstate.icon.flags & wimp_ICON_BG_COLOUR) >> wimp_ICON_BG_COLOUR_SHIFT) == wimp_COLOUR_WHITE)
    i++;

  iconstate.i = i - 2;
  wimp_get_icon_state(&iconstate);

  drag.type       = wimp_DRAG_ASM_RUBBER;

  drag.initial.x0 = pointer->pos.x;
  drag.initial.y0 = pointer->pos.y;
  drag.initial.x1 = pointer->pos.x;
  drag.initial.y1 = pointer->pos.y;

  /* icon bounds in screen area */
  drag.bbox.x0    = iconstate.icon.extent.x0 - x + GAP;
  drag.bbox.y0    = iconstate.icon.extent.y0 - y + GAP;
  drag.bbox.x1    = iconstate.icon.extent.x1 - x - GAP;
  drag.bbox.y1    = iconstate.icon.extent.y1 - y - GAP;

  drag.handle     = event_get_pollword();
  _swi(0x4D942 /* AppEngine_WindowOp */, _IN(0)|_OUTR(0,2),
       8, &drag.draw, &drag.undraw, &drag.redraw);

  slider.w        = pointer->w;
  slider.i        = i;
  slider.x        = x;
  slider.y        = y;

  /* icon bounds in work area */
  slider.bbox.x0  = iconstate.icon.extent.x0 + GAP;
  slider.bbox.y0  = iconstate.icon.extent.y0 + GAP;
  slider.bbox.x1  = iconstate.icon.extent.x1 - GAP;
  slider.bbox.y1  = iconstate.icon.extent.y1 - GAP;

  slider.callback = update;
  slider.val      = 708540049; // mad value for a default
  slider.min      = min;
  slider.max      = max;

  slider_set_handlers(1);

  wimp_drag_box(&drag);
}

/* 'i' should be the background 'pit' icon */
void slider_set(wimp_w w, wimp_i i, int val, int min, int max)
{
  wimp_icon_state icon;
  int             oldx;
  os_box          ext;
  int             newx;
  int             s_width;

  val -= min;
  max -= min;

  icon.w = w;
  icon.i = i + 2;
  wimp_get_icon_state(&icon);

  oldx = icon.icon.extent.x1;

  icon.i = i;
  wimp_get_icon_state(&icon);

  /* icon bounds in work area */
  ext.x0 = icon.icon.extent.x0 + GAP;
  ext.y0 = icon.icon.extent.y0 + GAP;
  ext.x1 = icon.icon.extent.x1 - GAP;
  ext.y1 = icon.icon.extent.y1 - GAP;

  s_width = ext.x1 - ext.x0;

  newx = (ext.x0 + (s_width - 1) * val / max) + 1;

  wimp_resize_icon(w, i + 2, ext.x0, ext.y0, newx, ext.y1);
  wimp_resize_icon(w, i + 1, newx, ext.y0, ext.x1, ext.y1);

  if (newx < oldx)
  {
    int temp;

    temp = oldx;
    oldx = newx;
    newx = temp;
  }

  wimp_force_redraw(w, oldx, ext.y0, newx, ext.y1);
}
