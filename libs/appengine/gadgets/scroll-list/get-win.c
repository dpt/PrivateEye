/* --------------------------------------------------------------------------
 *    Name: get-win.h
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

wimp_w scroll_list_get_window_handle(scroll_list *sl)
{
  return sl->w;
}
