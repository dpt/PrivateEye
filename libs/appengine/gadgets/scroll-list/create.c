/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

static const wimp_window wdef =
{
  { 0, 0, 0, 0 }, /* visible */
  0, 0,
  wimp_TOP,
  wimp_WINDOW_VSCROLL | wimp_WINDOW_NEW_FORMAT,
  wimp_COLOUR_BLACK,
  wimp_COLOUR_LIGHT_GREY,
  wimp_COLOUR_BLACK,
  wimp_COLOUR_TRANSPARENT,
  wimp_COLOUR_MID_LIGHT_GREY,
  wimp_COLOUR_VERY_LIGHT_GREY,
  wimp_COLOUR_CREAM,
  0,
  { 0, 0, 0, 0 }, /* extent */
  wimp_ICON_TEXT | wimp_ICON_HCENTRED | wimp_ICON_VCENTRED | wimp_ICON_INDIRECTED,
  wimp_BUTTON_WRITE_CLICK_DRAG << wimp_ICON_BUTTON_TYPE_SHIFT,
  wimpspriteop_AREA,
  0, 0,
  { .indirected_text = { "", "", 1 } },
  0,
};

/* ----------------------------------------------------------------------- */

scroll_list *scroll_list__create(wimp_w main_w, wimp_i main_i)
{
  scroll_list      *sl;
  wimp_window_state wstate;
  wimp_icon_state   istate;
  int               x,y;
  int               w,h;
  wimp_window       def;
  wimp_w            sw;

  wstate.w = main_w;
  wimp_get_window_state(&wstate);

  istate.w = main_w;
  istate.i = main_i;
  wimp_get_icon_state(&istate);

  x = wstate.visible.x0 - wstate.xscroll;
  y = wstate.visible.y1 - wstate.yscroll;

  w = istate.icon.extent.x1 - istate.icon.extent.x0;
  h = istate.icon.extent.y1 - istate.icon.extent.y0;

  def = wdef;

  def.extent.x0 = 0;
  def.extent.y0 = -h;
  def.extent.x1 = w;
  def.extent.y1 = 0;

  def.xmin = w;
  def.ymin = h;

  sw = wimp_create_window(&def);

  wstate.w = sw;
  wstate.visible.x0 = x + istate.icon.extent.x0;
  wstate.visible.y0 = y + istate.icon.extent.y0;
  wstate.visible.x1 = x + istate.icon.extent.x1;
  wstate.visible.y1 = y + istate.icon.extent.y1;

  wimp_open_window_nested((wimp_open *) &wstate,
                          main_w,
                          0 /* all link work area */);

  sl = malloc(sizeof(*sl));
  if (sl == NULL)
  {
    // cleanup
    return sl;
  }

  sl->w         = sw;
  sl->width     = w;
  sl->height    = 44;
  sl->leading   = 0;
  sl->rows      = 0;
  sl->selection = -1; /* no initial selection */
  sl->marker    = -1; /* no initial marker */

  scroll_list__internal_set_handlers(1, sw, sl);

  return sl;
}

