/* --------------------------------------------------------------------------
 *    Name: set-tags.h
 * Purpose: Tag cloud
 * Version: $Id: set-tags.c,v 1.3 2010-06-02 21:58:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/dict.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/messages.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"
#include "iconnames.h"

static void calc_scales(tag_cloud *tc)
{
  int min, max;
  int i;
  int scale;

  /* work out tag display scaling */

  min = INT_MAX;
  max = INT_MIN;

  for (i = 0; i < tc->e_used; i++)
  {
    int s;

    s = tc->entries[i].count;

    if (s < min)
      min = s;
    /* don't put an else here */
    if (s > max)
      max = s;
  }

  tc->scale.min = min;

  /* this integer division is intentional to create 'steps' */

  scale = (max - min) / 5;
  if (scale <= 0)
    scale = 1;

  tc->scale.scale = scale;
}

error tag_cloud__set_tags(tag_cloud            *tc,
                          const tag_cloud__tag *tags,
                          int                   ntags)
{
  error                 err;
  dict_t               *dict        = NULL;
  int                   totalcount;
  const tag_cloud__tag *t;
  tag_cloud__entry     *entries     = NULL;
  int                   e_used      = 0;
  int                   e_allocated = 0;
  void                 *newarr;
  int                  *sorted;
  int                   i;

  dict__destroy(tc->dict);
  tc->dict = NULL;

  free(tc->entries);
  tc->entries     = NULL;
  tc->e_used      = 0;
  tc->e_allocated = 0;

  /* populate a dictionary with the new tags and at the same time construct
   * a parallel array containing dictionary indices and tag counts. the
   * array entries can be re-ordered to sort the data.
   */

  dict = dict__create();
  if (dict == NULL)
    return error_OOM;

  totalcount = 0;

  for (t = tags; t < tags + ntags; t++)
  {
    dict_index index;

    err = dict__add(dict, t->name, &index);
    if (err != error_DICT_NAME_EXISTS && err)
      return err;

    if (index < e_used)
    {
      /* duplicate tag */

      /* add the duplicate's count to the original's */
      entries[index].count += t->count;
    }
    else
    {
      /* new tag */

      if (index >= e_allocated) /* need more space? */
      {
        size_t n;

        n = (size_t) power2gt(index);

        newarr = realloc(entries, n * sizeof(*entries));
        if (newarr == NULL)
        {
          err = error_OOM;
          goto Failure;
        }

        entries     = newarr;
        e_allocated = n;
      }

      entries[index].count = t->count;
      e_used++;
    }

    totalcount += t->count;
  }

  /* we assume that the returned indices cover the whole range, so don't
   * leave us with any empty entries */

  /* shrink wrap */
  newarr = realloc(entries, e_used * sizeof(*entries));
  if (newarr == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  entries = newarr;


  /* set up index table (which we use when sorting) */

  sorted = malloc(e_used * sizeof(*sorted));
  if (sorted == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  for (i = 0; i < e_used; i++)
    sorted[i] = i;


  tc->dict        = dict;

  tc->entries     = entries;
  tc->e_used      = e_used;
  tc->e_allocated = e_used;

  free(tc->sorted);
  tc->sorted      = sorted;

  calc_scales(tc);

  tc->flags |= tag_cloud__FLAG_NEW_DATA;


  tag_cloud__set_sort(tc, tc->sort_type); /* kick */

  tag_cloud__schedule_redraw(tc);

  tc->menued_tag_index = -1; // not pleasant doing this here


  if (tc->flags & tag_cloud__FLAG_TOOLBAR)
  {
    /* set up the stats icon */

    icon_printf(tc->toolbar_w, TAG_CLOUD_T_D_INFO,
                message0("tagcloud.stats"), ntags, totalcount);
  }

  return error_OK;

Failure:

  dict__destroy(dict);

  free(entries);

  return err;
}
