/* --------------------------------------------------------------------------
 *    Name: where-insert.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

/* This returns an insertion point, as opposed to a row. */
int scroll_list__where_to_insert(scroll_list *sl, wimp_pointer *pointer)
{
  wimp_window_state state;
  int               y;
  int               i;

  state.w = sl->w;
  wimp_get_window_state(&state);

  y = pointer->pos.y + (state.yscroll - state.visible.y1);

  y -= (sl->height + sl->leading) / 2;

  i = wa_to_row(sl, y);

  /* this isn't the same as clamp() */

  if (i < 0)
    return 0;

  if (i > sl->rows)
    return sl->rows;

  return i;
}

