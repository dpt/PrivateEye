/* --------------------------------------------------------------------------
 *    Name: open.c
 * Purpose: Tag cloud
 * Version: $Id: open.c,v 1.2 2010-06-02 21:58:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

static void kick(tag_cloud *tc, wimp_open *open, os_box *box)
{
  int width;

  width = open->visible.x1 - open->visible.x0;

  /* only layout + redraw where necessary */
  if (tc->layout.width != width ||
      tc->flags & (tag_cloud__FLAG_NEW_DATA | tag_cloud__FLAG_NEW_HIGHLIGHTS)) // probably other conditions too
  {
    tag_cloud__layout(tc, width);

    box->x0 = 0;
    box->y0 = -tc->layout.height - 44; /* add a bit of extra vertical space */
    box->x1 = 16384;
    box->y1 = (tc->flags & tag_cloud__FLAG_TOOLBAR) ? 64 : 0; /* height of toolbar window */
    wimp_set_extent(tc->main_w, box);

    wimp_open_window(open); // have to re-kick after extent change
  }
}

void tag_cloud__post_open(tag_cloud *tc, wimp_open *open)
{
  os_box box;

  kick(tc, open, &box);
}

void tag_cloud__post_reopen(tag_cloud *tc, wimp_open *open)
{
  os_box box;

  kick(tc, open, &box);

  /* immediate redraw */
  wimp_force_redraw(tc->main_w, box.x0, box.y0, box.x1, box.y1);
}

void tag_cloud__open(tag_cloud *tc)
{
  wimp_window_state state;

  state.w = tc->main_w;
  wimp_get_window_state(&state);

  state.next = wimp_TOP;
  wimp_open_window((wimp_open *) &state);

  tag_cloud__post_open(tc, (wimp_open *) &state);
}
