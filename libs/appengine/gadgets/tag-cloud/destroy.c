/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Tag cloud
 * Version: $Id: destroy.c,v 1.2 2010-06-02 21:58:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

void tag_cloud__destroy(tag_cloud *doomed)
{
  if (doomed == NULL)
    return;

  free(doomed->highlight.indices);

  tag_cloud__layout_discard(doomed);

  free(doomed->sorted);
  free(doomed->entries);
  dict__destroy(doomed->dict);

  tag_cloud__internal_set_handlers(0, doomed);

  tag_cloud__detach_toolbar(doomed);

  window_delete_cloned(doomed->main_w);

  free(doomed);
}
