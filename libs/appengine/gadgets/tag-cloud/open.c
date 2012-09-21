/* --------------------------------------------------------------------------
 *    Name: open.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

void tag_cloud_post_reopen(tag_cloud *tc)
{
  tag_cloud_sync(tc, tag_cloud_SYNC_REDRAW_IF_LAYOUT);
}

void tag_cloud_open(tag_cloud *tc)
{
  tag_cloud_sync(tc, tag_cloud_SYNC_OPEN_TOP);
}

/* ----------------------------------------------------------------------- */
