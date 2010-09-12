/* --------------------------------------------------------------------------
 *    Name: set-row-height.c
 * Purpose: Scrolling list
 * Version: $Id: set-row-height.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__set_row_height(scroll_list *sl, int height, int leading)
{
  sl->height  = height;
  sl->leading = leading;
}
