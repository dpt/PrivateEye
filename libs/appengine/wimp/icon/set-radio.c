/* $Id: set-radio.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_set_radio(wimp_w w, wimp_i i)
{
  wimp_icon_state state;
  wimp_i          all[16]; /* oversized for safety */
  wimp_i          prev;

  state.w = w;
  state.i = i;
  wimp_get_icon_state(&state);

  /* which icons are on and shouldn't be on? turn them off */

  /* find the previously selected icon wih the same ESG */

  wimp_which_icon(w, all,
                  wimp_ICON_SELECTED | wimp_ICON_ESG,
                  wimp_ICON_SELECTED | (state.icon.flags & wimp_ICON_ESG));

  prev = all[0];

  if (prev == i) /* already selected? */
    return; /* assumes no others are selected either */

  if (prev != -1) /* nothing selected */
    icon_set_selected(w, prev, 0);

  icon_set_selected(w, i, 1);
}
