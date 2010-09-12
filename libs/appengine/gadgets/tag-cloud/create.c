/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Tag cloud
 * Version: $Id: create.c,v 1.2 2010-06-02 21:58:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

tag_cloud *tag_cloud__create(tag_cloud__create_flags  flags,
                             const tag_cloud__config *config)
{
  tag_cloud *tc;
  wimp_w     main_w;

  tc = calloc(1, sizeof(*tc));
  if (tc == NULL)
    return tc;

  main_w = window_clone(tag_cloud__get_main_window());

  tc->config           = *config;

  tc->main_w           = main_w;

  tc->sort_type        = 0;

  tc->menued_tag_index = -1;

  tc->sort.last_sort_type  = -1;
  tc->sort.last_order_type = -1;

  tag_cloud__set_display(tc, tag_cloud__DISPLAY_TYPE_CLOUD);

  tag_cloud__internal_set_handlers(1, tc);

  if (flags & tag_cloud__CREATE_FLAG_TOOLBAR_DISABLED)
  {
    tc->flags |= tag_cloud__FLAG_TOOLBAR_NOT_EVER;
  }
  else if ((flags & tag_cloud__CREATE_FLAG_TOOLBAR_HIDDEN) == 0)
  {
    tag_cloud__attach_toolbar(tc);
  }

  return tc;
}
