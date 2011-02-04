/* --------------------------------------------------------------------------
 *    Name: set-handlers.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list__set_handlers(scroll_list           *sl,
                               scroll_list__redrawfn *redraw_elem,
                               scroll_list__redrawfn *redraw_lead,
                               scroll_list__eventfn  *event)
{
  sl->redraw_elem = redraw_elem;
  sl->redraw_lead = redraw_lead;
  sl->event       = event;
}
