/* --------------------------------------------------------------------------
 *    Name: scaling.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "appengine/types.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/atom.h"

#include "appengine/gadgets/tag-cloud.h"

#include "iconnames.h"
#include "impl.h"

static void redisplay(tag_cloud *tc)
{
  tc->flags |= tag_cloud_FLAG_NEW_DISPLAY;
  tag_cloud_redraw(tc);
}

/* ----------------------------------------------------------------------- */

static void tag_cloud_kick_scaling_icon(tag_cloud *tc)
{
  if (tc->flags & tag_cloud_FLAG_TOOLBAR)
    /* toolbar icons must match the order of scaling_type */
    icon_set_radio(tc->toolbar_w, TAG_CLOUD_T_B_SCALEOFF + tc->scaling_type);
}

void tag_cloud_set_scaling(tag_cloud *tc, int scaling_type)
{
  if ((unsigned int) scaling_type >= tag_cloud_SCALING__LIMIT)
    scaling_type = 0;

  /* avoid updating the display wherever possible */

  if (tc->scaling_type == scaling_type)
    return;

  tc->scaling_type = scaling_type;

  redisplay(tc);

  tag_cloud_kick_scaling_icon(tc);
}

int tag_cloud_get_scaling(tag_cloud *tc)
{
  return tc->scaling_type;
}
