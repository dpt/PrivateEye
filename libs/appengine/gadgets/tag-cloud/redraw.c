/* --------------------------------------------------------------------------
 *    Name: redraw.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

void tag_cloud_sync(tag_cloud *tc, tag_cloud_sync_flags flags)
{
  wimp_window_state state;
  int               visible_width;
  int               old_width, old_height;

  state.w = tc->main_w;
  wimp_get_window_state(&state);

  /* do nothing if the window is closed */

  if ((flags & tag_cloud_SYNC_OPEN_TOP) == 0 &&
      (state.flags & wimp_WINDOW_OPEN) == 0)
    return;

  /* layout to fit the visible width */

  visible_width = state.visible.x1 - state.visible.x0;

  old_width  = tc->layout.width;
  old_height = tc->layout.height;

  (void) tag_cloud_layout(tc, visible_width); // error ignored

  /* set extent if the vertical size has changed */

  if ((flags & (tag_cloud_SYNC_OPEN_TOP | tag_cloud_SYNC_EXTENT)) != 0 ||
      tc->layout.height != old_height)
  {
    os_box box;

    box.x0 = 0;
    box.y0 = -tc->layout.height;
    box.x1 = 16384;
    box.y1 = 0;

    tag_cloud_toolbar_adjust_extent(tc, &box);

    wimp_set_extent(tc->main_w, &box);

    if (flags & tag_cloud_SYNC_OPEN_TOP)
      state.next = wimp_TOP;

    /* have to re-kick after extent change */
    wimp_open_window((wimp_open *) &state);
  }

  if ((flags & tag_cloud_SYNC_REDRAW) != 0 ||
      ((flags & tag_cloud_SYNC_REDRAW_IF_LAYOUT) != 0 && (tc->layout.width != old_width)))
    wimp_force_redraw(tc->main_w, 0, -16384, 16384, 0);
}

/* ----------------------------------------------------------------------- */

void tag_cloud_redraw(tag_cloud *tc)
{
  tag_cloud_sync(tc, tag_cloud_SYNC_REDRAW);
}

void tag_cloud_kick_extent(tag_cloud *tc)
{
  tag_cloud_sync(tc, tag_cloud_SYNC_EXTENT);
}

/* ----------------------------------------------------------------------- */
