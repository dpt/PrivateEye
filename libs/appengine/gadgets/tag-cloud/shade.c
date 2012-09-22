/* --------------------------------------------------------------------------
 *    Name: shade.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

void tag_cloud_shade(tag_cloud *tc, int shade)
{
  if (shade)
    tc->flags |= tag_cloud_FLAG_SHADED;
  else
    tc->flags &= ~tag_cloud_FLAG_SHADED;

  tc->flags |= tag_cloud_FLAG_NEW_SHADE;

  tag_cloud_redraw(tc);
}
