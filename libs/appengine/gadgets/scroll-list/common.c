/* --------------------------------------------------------------------------
 *    Name: common.c
 * Purpose: Scrolling list
 * Version: $Id: common.c,v 1.2 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

int row_to_wa(scroll_list *sl, int row)
{
  return -(row * (sl->height + sl->leading));
}

int wa_to_row(scroll_list *sl, int x)
{
  return -x / (sl->height + sl->leading);
}

/* ----------------------------------------------------------------------- */

/* bounds check */
int clamp(scroll_list *sl, int row)
{
  if (row < 0)
    return 0;
  else if (row > sl->rows - 1)
    return sl->rows - 1;
  else
    return row;
}

/* ----------------------------------------------------------------------- */

void refresh_rows(scroll_list *sl, int min, int max)
{
  int y1;
  int y0;

  y1 = row_to_wa(sl, min);
  y0 = row_to_wa(sl, max + 1);

  wimp_force_redraw(sl->w, 0, y0, sl->width, y1);
}

/* ----------------------------------------------------------------------- */

void resize_pane(scroll_list *sl)
{
  wimp_window_info info;

  info.w = sl->w;
  wimp_get_window_info_header_only(&info);

  info.extent.y1 = 0;
  info.extent.y0 = row_to_wa(sl, sl->rows) - sl->leading;
  wimp_set_extent(sl->w, &info.extent);

  /* set_extent doesn't automatically adjust the window. kick it. */
  wimp_open_window((wimp_open *) &info);
}

/* ----------------------------------------------------------------------- */

/* This is only going to cope with selection-based events. */
void send_event(scroll_list *sl, scroll_list__event_type type)
{
  scroll_list__event event;

  event.type  = type;
  event.index = scroll_list__get_selection(sl);

  sl->event(&event);
}

