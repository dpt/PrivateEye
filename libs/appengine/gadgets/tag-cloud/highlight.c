/* --------------------------------------------------------------------------
 *    Name: highlight.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/base/bsearch.h"
#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

error tag_cloud__highlight(tag_cloud *tc, int *indices, int nindices)
{
  free(tc->highlight.indices);

  if (nindices == 0)
  {
    tc->highlight.indices  = NULL;
    tc->highlight.nindices = 0;
  }
  else
  {
    int *newindices;

    newindices = malloc(nindices * sizeof(*indices));
    if (newindices == NULL)
      return error_OOM;

    memcpy(newindices, indices, nindices * sizeof(*indices));

    tc->highlight.indices  = newindices;
    tc->highlight.nindices = nindices;
  }

  tc->flags |= tag_cloud__FLAG_NEW_HIGHLIGHTS;

  if (tc->flags & tag_cloud__FLAG_SORT_SEL_FIRST)
    tag_cloud__set_sort(tc, tc->sort_type); /* kick the sorter */

  tag_cloud__schedule_redraw(tc);

  return error_OK;
}

int tag_cloud__is_highlighted(tag_cloud *tc, int index)
{
  if (tc->highlight.nindices == 0)
    return 0;

  return bsearch_int(tc->highlight.indices,
                     tc->highlight.nindices,
                     sizeof(*tc->highlight.indices),
                     index) > -1;
}
