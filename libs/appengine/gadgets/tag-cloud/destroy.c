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

void tag_cloud__destroy(tag_cloud *doomed)
{
  if (doomed == NULL)
    return;

  bitvec__destroy(doomed->highlight);

  tag_cloud__layout_discard(doomed);

  free(doomed->sorted);
  free(doomed->entries);
  atom_destroy(doomed->dict);

  tag_cloud__internal_set_handlers(0, doomed);

  tag_cloud__detach_toolbar(doomed);

  window_delete_cloned(doomed->main_w);

  free(doomed);
}
