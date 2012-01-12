/* --------------------------------------------------------------------------
 *    Name: refresh-all.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list_refresh_all_rows(scroll_list *sl)
{
  refresh_rows(sl, 0, sl->rows);
}
