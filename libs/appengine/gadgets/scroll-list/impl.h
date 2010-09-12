/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Scrolling list
 * Version: $Id: impl.h,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef SCROLL_LIST_IMPL_H
#define SCROLL_LIST_IMPL_H

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

struct scroll_list
{
  wimp_w                 w;
  int                    width;
  int                    height;
  int                    leading;
  int                    rows;
  int                    selection;
  int                    marker;
  scroll_list__redrawfn *redraw_elem;
  scroll_list__redrawfn *redraw_lead;
  scroll_list__eventfn  *event;
};

int row_to_wa(scroll_list *sl, int row);
int wa_to_row(scroll_list *sl, int x);
int clamp(scroll_list *sl, int row);
void refresh_rows(scroll_list *sl, int min, int max);
void resize_pane(scroll_list *sl);
void send_event(scroll_list *sl, scroll_list__event_type type);

void scroll_list__internal_set_handlers(int reg, wimp_w w, scroll_list *sl);

#endif /* SCROLL_LIST_IMPL_H */
