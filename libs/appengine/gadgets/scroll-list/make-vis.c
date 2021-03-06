/* --------------------------------------------------------------------------
 *    Name: make-vis.h
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list_make_visible(scroll_list *sl, int row)
{
  wimp_window_state state;

  /* FIXME: if (row outside visible portion of window) { scroll } */
  state.w = sl->w;
  wimp_get_window_state(&state);
  state.yscroll = row_to_wa(sl, row) + sl->leading;
  wimp_open_window((wimp_open *) &state);
}
