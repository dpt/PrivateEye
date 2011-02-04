/* --------------------------------------------------------------------------
 *    Name: shade.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

void tag_cloud__shade(tag_cloud *tc, int shade)
{
  if (shade)
    tc->flags |= tag_cloud__FLAG_SHADED;
  else
    tc->flags &= ~tag_cloud__FLAG_SHADED;
}
