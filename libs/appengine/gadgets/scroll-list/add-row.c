/* --------------------------------------------------------------------------
 *    Name: add-row.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

// need insert_row?
void scroll_list_add_row(scroll_list *sl)
{
  refresh_rows(sl, sl->rows, sl->rows);

  sl->rows++;

  resize_pane(sl);
}
