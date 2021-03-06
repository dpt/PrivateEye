/* --------------------------------------------------------------------------
 *    Name: highlight.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "datastruct/bitvec.h"

#include "appengine/base/bsearch.h"
#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

static void tag_cloud_kick_highlight(tag_cloud *tc)
{
  tc->flags |= tag_cloud_FLAG_NEW_HIGHLIGHTS;

  if (tc->flags & tag_cloud_FLAG_SORT_SEL_FIRST)
    tag_cloud_set_sort(tc, tc->sort_type); /* kick the sorter */

  tag_cloud_redraw(tc);
}

static result_t tag_cloud_add_highlights(tag_cloud *tc,
                                         const int *indices,
                                         int        nindices)
{
  result_t err;
  int      i;

  if (tc->highlight == NULL)
  {
    tc->highlight = bitvec_create(32); /* 32 is a guess */
    if (tc->highlight == NULL)
      return result_OOM;
  }

  for (i = 0; i < nindices; i++)
  {
    err = bitvec_set(tc->highlight, indices[i]);
    if (err)
      return err;
  }

  tag_cloud_kick_highlight(tc);

  return result_OK;
}

result_t tag_cloud_highlight(tag_cloud *tc, const int *indices, int nindices)
{
  result_t err;

  if (tc->highlight)
    bitvec_clear_all(tc->highlight);

  err = tag_cloud_add_highlights(tc, indices, nindices);
  if (err)
    return err;

  return result_OK;
}

result_t tag_cloud_add_highlight(tag_cloud *tc, int index)
{
  result_t err;

  err = tag_cloud_add_highlights(tc, &index, 1);
  if (err)
    return err;

  return result_OK;
}

void tag_cloud_remove_highlight(tag_cloud *tc, int index)
{
  bitvec_clear(tc->highlight, index);

  tag_cloud_kick_highlight(tc);
}

int tag_cloud_is_highlighted(tag_cloud *tc, int index)
{
  if (tc->highlight == NULL)
    return 0;

  return bitvec_get(tc->highlight, index);
}
