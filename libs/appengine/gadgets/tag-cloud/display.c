/* --------------------------------------------------------------------------
 *    Name: display.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "appengine/types.h"
#include "appengine/wimp/icon.h"
#include "appengine/gadgets/tag-cloud.h"

#include "iconnames.h"
#include "impl.h"

static void tag_cloud_kick_display_icon(tag_cloud *tc)
{
  if (tc->toolbar_w)
    /* toolbar icons must match the order of display_type */
    icon_set_radio(tc->toolbar_w, TAG_CLOUD_T_B_DISPLIST + tc->display_type);
}

void tag_cloud_set_display(tag_cloud *tc, int display_type)
{
  assert(display_type < tag_cloud_DISPLAY_TYPE__LIMIT);

  tc->flags |= tag_cloud_FLAG_NEW_DISPLAY;

  tc->display_type = display_type;

  tag_cloud_schedule_redraw(tc);

  tag_cloud_kick_display_icon(tc);
}

int tag_cloud_get_display(tag_cloud *tc)
{
  return tc->display_type;
}
