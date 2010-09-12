/* --------------------------------------------------------------------------
 *    Name: which.c
 * Purpose: Scrolling list
 * Version: $Id: which.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

int scroll_list__which(scroll_list *sl, wimp_pointer *pointer)
{
  wimp_window_state state;
  int               y;
  int               i;

  state.w = sl->w;
  wimp_get_window_state(&state);

  y = pointer->pos.y + (state.yscroll - state.visible.y1);

  i = wa_to_row(sl, y);
  if (i < 0 || i >= sl->rows)
    return -1;

  return i;
}
