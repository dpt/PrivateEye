/* --------------------------------------------------------------------------
 *    Name: get-win.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

wimp_w tag_cloud_get_window_handle(tag_cloud *tc)
{
  return tc->main_w;
}
