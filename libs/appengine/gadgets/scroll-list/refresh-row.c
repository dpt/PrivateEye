/* --------------------------------------------------------------------------
 *    Name: refresh-row.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__refresh_row(scroll_list *sl, int row)
{
  int y1;
  int y0;

  if (row < 0)
    return;

  /* allow rows past the end so the trailing leader can be refreshed */

  /* refresh the row including the leading */

  y1 = row_to_wa(sl, row);
  y0 = y1 - (sl->height + sl->leading);

  wimp_force_redraw(sl->w, 0, y0, sl->width, y1);
}
