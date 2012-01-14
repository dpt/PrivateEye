/* --------------------------------------------------------------------------
 *    Name: set-config.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

error tag_cloud_set_config(tag_cloud        *tc,
                     const tag_cloud_config *config)
{
  tc->config = *config;

  tag_cloud_layout_discard(tc);

  tc->flags |= tag_cloud_FLAG_NEW_DISPLAY;

  tag_cloud_schedule_redraw(tc);

  return error_OK;
}
