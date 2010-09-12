/* --------------------------------------------------------------------------
 *    Name: refresh-all.c
 * Purpose: Scrolling list
 * Version: $Id: refresh-all.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__refresh_all_rows(scroll_list *sl)
{
  refresh_rows(sl, 0, sl->rows);
}
