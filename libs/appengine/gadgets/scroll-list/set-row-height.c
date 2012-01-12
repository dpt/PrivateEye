/* --------------------------------------------------------------------------
 *    Name: set-row-height.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list_set_row_height(scroll_list *sl, int height, int leading)
{
  sl->height  = height;
  sl->leading = leading;
}
