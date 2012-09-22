/* --------------------------------------------------------------------------
 *    Name: toolbar.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "iconnames.h"
#include "impl.h"

/* ----------------------------------------------------------------------- */

void tag_cloud_attach_toolbar(tag_cloud *tc)
{
  wimp_w                    toolbar_w;
  wimp_window_state         wstate;
  wimp_window_nesting_flags linkage;

  if (tc->flags & tag_cloud_FLAG_TOOLBAR)
    return; /* already attached */

  /* turn off all event handlers */
  // this is grisly - maybe have tag_cloud_internal_set_handlers_toolbar?
  tag_cloud_internal_set_handlers(0, tc);

  toolbar_w = window_clone(tag_cloud_get_toolbar_window());

  wstate.w = tc->main_w;
  wimp_get_window_state(&wstate);

  wstate.w = toolbar_w;

  linkage = (wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_LS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_BS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT   << wimp_CHILD_RS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT   << wimp_CHILD_TS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_VISIBLE_BOTTOM_OR_LEFT << wimp_CHILD_XORIGIN_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_VISIBLE_TOP_OR_RIGHT   << wimp_CHILD_YORIGIN_SHIFT);

  wimp_open_window_nested((wimp_open *) &wstate, tc->main_w, linkage);

  tc->flags    |= tag_cloud_FLAG_TOOLBAR;
  tc->toolbar_w = toolbar_w;

  tag_cloud_internal_set_handlers(1, tc);

  // reset the buttons - also grisly, copied from display.c etc.
  // have separate calls for these too? e.g. tag_cloud_set_display_sync_toolbar?

  /* toolbar icons need to match the order of display_type */
  icon_set_radio(tc->toolbar_w, TAG_CLOUD_T_B_DISPLIST + tc->display_type);

  /* toolbar icons need to match the order of sort_type */
  icon_set_radio(tc->toolbar_w, TAG_CLOUD_T_B_SORTALPHA + tc->sort_type);

  icon_set_selected(tc->toolbar_w, TAG_CLOUD_T_O_SELFIRST,
                    tc->flags & tag_cloud_FLAG_SORT_SEL_FIRST);
}

void tag_cloud_detach_toolbar(tag_cloud *tc)
{
  if ((tc->flags & tag_cloud_FLAG_TOOLBAR) == 0)
    return; /* already detached */

  /* turn off all event handlers */
  // this is grisly - see above
  tag_cloud_internal_set_handlers(0, tc);

  window_delete_cloned(tc->toolbar_w);

  tc->flags    &= ~tag_cloud_FLAG_TOOLBAR;
  tc->toolbar_w = NULL;

  tag_cloud_internal_set_handlers(1, tc);
}

void tag_cloud_toggle_toolbar(tag_cloud *tc)
{
  if (tc->flags & tag_cloud_FLAG_TOOLBAR)
    tag_cloud_detach_toolbar(tc);
  else
    tag_cloud_attach_toolbar(tc);

  tag_cloud_kick_extent(tc);
}

/* ----------------------------------------------------------------------- */

void tag_cloud_toolbar_adjust_extent(const tag_cloud *tc, os_box *box)
{
  if (tc->flags & tag_cloud_FLAG_TOOLBAR)
    box->y1 += 68; /* height of toolbar window */
}

/* ----------------------------------------------------------------------- */
