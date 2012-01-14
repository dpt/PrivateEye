/* --------------------------------------------------------------------------
 *    Name: create.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

tag_cloud *tag_cloud_create(tag_cloud_create_flags  flags,
                            const tag_cloud_config *config)
{
  tag_cloud *tc;
  wimp_w     main_w;

  tc = calloc(1, sizeof(*tc));
  if (tc == NULL)
    return tc;

  main_w = window_clone(tag_cloud_get_main_window());

  tc->config               = *config;
  tc->main_w               = main_w;
  tc->sort_type            = 0;
  tc->menued_tag_index     = -1;
  tc->sort.last_sort_type  = -1;
  tc->sort.last_order_type = -1;

  tag_cloud_set_display(tc, tag_cloud_DISPLAY_TYPE_CLOUD);

  tag_cloud_internal_set_handlers(1, tc);

  if (flags & tag_cloud_CREATE_FLAG_TOOLBAR_DISABLED)
    tc->flags |= tag_cloud_FLAG_TOOLBAR_NOT_EVER;
  else if ((flags & tag_cloud_CREATE_FLAG_TOOLBAR_HIDDEN) == 0)
    tag_cloud_attach_toolbar(tc);

  return tc;
}
