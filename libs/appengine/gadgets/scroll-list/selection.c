/* --------------------------------------------------------------------------
 *    Name: selection.c
 * Purpose: Scrolling list
 * Version: $Id: selection.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__set_selection(scroll_list *sl, int row)
{
  int old;
  int sel;

  old = sl->selection;

  sel = clamp(sl, row);

  if (sel != old)
  {
    sl->selection = sel;

    scroll_list__refresh_row(sl, old);
    scroll_list__refresh_row(sl, sel);
  }
}

int scroll_list__get_selection(scroll_list *sl)
{
  assert(sl->selection >= -1);
  assert(sl->selection <= sl->rows - 1);

  return sl->selection;
}

void scroll_list__clear_selection(scroll_list *sl)
{
  int old;

  old = sl->selection;

  sl->selection = -1;

  scroll_list__refresh_row(sl, old);
}

int scroll_list__move_selection_absolute(scroll_list *sl, int where)
{
  int old;
  int sel;

  old = sl->selection;

  if (old == -1)
    return -1; /* no selection */

  sel = where;
  sel = clamp(sl, sel);

  if (sel != old)
    scroll_list__set_selection(sl, sel);

  return sel; /* the new selection */
}

int scroll_list__move_selection_relative(scroll_list *sl, int delta)
{
  return scroll_list__move_selection_absolute(sl, sl->selection + delta);
}
