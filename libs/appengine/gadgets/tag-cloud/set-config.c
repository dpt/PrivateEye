/* --------------------------------------------------------------------------
 *    Name: set-config.c
 * Purpose: Tag cloud
 * Version: $Id: set-config.c,v 1.2 2010-06-02 21:58:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

error tag_cloud__set_config(tag_cloud         *tc,
                      const tag_cloud__config *config)
{
  tc->config = *config;

  tag_cloud__layout_discard(tc);

  tc->flags |= tag_cloud__FLAG_NEW_DISPLAY;

  tag_cloud__schedule_redraw(tc);

  return error_OK;
}
