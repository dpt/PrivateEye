/* --------------------------------------------------------------------------
 *    Name: set-handlers.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list_set_handlers(scroll_list          *sl,
                              scroll_list_redrawfn *redraw_elem,
                              scroll_list_redrawfn *redraw_lead,
                              scroll_list_eventfn  *event,
                              void                 *opaque)
{
  sl->redraw_elem = redraw_elem;
  sl->redraw_lead = redraw_lead;
  sl->event       = event;
  sl->opaque      = opaque;
}
