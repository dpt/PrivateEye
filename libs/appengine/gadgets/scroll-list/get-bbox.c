/* --------------------------------------------------------------------------
 *    Name: getbbox.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__get_bbox(scroll_list *sl, int row, os_box *box)
{
  int y1;

  y1 = row_to_wa(sl, row);

  box->x0 = 0;
  box->y0 = y1 - (sl->height + sl->leading);
  box->x1 = sl->width;
  box->y1 = y1;
}
