/* --------------------------------------------------------------------------
 *    Name: add-row.c
 * Purpose: Scrolling list
 * Version: $Id: add-row.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

// need insert_row?
void scroll_list__add_row(scroll_list *sl)
{
  refresh_rows(sl, sl->rows, sl->rows);

  sl->rows++;

  resize_pane(sl);
}
