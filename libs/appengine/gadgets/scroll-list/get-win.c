/* --------------------------------------------------------------------------
 *    Name: get-win.h
 * Purpose: Scrolling list
 * Version: $Id: get-win.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

wimp_w scroll_list__get_window_handle(scroll_list *sl)
{
  return sl->w;
}
