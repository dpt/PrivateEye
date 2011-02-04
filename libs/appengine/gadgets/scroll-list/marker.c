/* --------------------------------------------------------------------------
 *    Name: marker.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__set_marker(scroll_list *sl, int where)
{
  assert(where >= 0);
  assert(where <= sl->rows);

  scroll_list__refresh_row(sl, sl->marker);
  scroll_list__refresh_row(sl, where);

  sl->marker = where;
}

void scroll_list__clear_marker(scroll_list *sl)
{
  scroll_list__refresh_row(sl, sl->marker);
  sl->marker = -1;
}
