/* --------------------------------------------------------------------------
 *    Name: get-win.h
 * Purpose: Tag cloud
 * Version: $Id: get-win.c,v 1.1 2010-01-05 22:39:36 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

wimp_w tag_cloud__get_window_handle(tag_cloud *tc)
{
  return tc->main_w;
}
