/* --------------------------------------------------------------------------
 *    Name: delete-rows.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

/* max is inclusive */
void scroll_list_delete_rows(scroll_list *sl, int min, int max)
{
  int n;

  min = clamp(sl, min);
  max = clamp(sl, max);

  if (max == -1)
    return; /* no rows */

  /* refresh all subsequent rows
   * do this now before we start moving things around */
  refresh_rows(sl, min, sl->rows - 1 /* NOT max! */);

  n = max + 1 - min;

  sl->rows -= n;

  resize_pane(sl);

  if (sl->selection >= min && sl->selection <= max)
  {
    sl->selection = -1; /* delete selection - associated row deleted */
  }
  else
  {
    if (sl->selection >= max)
      sl->selection -= n;
  }
}

