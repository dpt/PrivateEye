/* --------------------------------------------------------------------------
 *    Name: destroy.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

void tag_cloud_destroy(tag_cloud *doomed)
{
  if (doomed == NULL)
    return;

  bitvec_destroy(doomed->highlight);

  tag_cloud_layout_discard(doomed);

  free(doomed->sorted);
  free(doomed->entries);
  atom_destroy(doomed->dict);

  tag_cloud_internal_set_handlers(0, doomed);

  tag_cloud_detach_toolbar(doomed);

  window_delete_cloned(doomed->main_w);

  free(doomed);
}
