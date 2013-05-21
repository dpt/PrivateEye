/* --------------------------------------------------------------------------
 *    Name: set-tags.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/atom.h"
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

  for (i = 0; i < tc->ntags; i++)
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

error tag_cloud_set_tags(tag_cloud           *tc,
                         const tag_cloud_tag *tags,
                         int                  ntags)
{
  error                err;
  atom_set_t          *dict        = NULL;
  tag_cloud_entry     *entries     = NULL;
  int                  totalcount;
  int                  i;
  int                 *sorted      = NULL;

  /* clean out any previous tags */

  atom_destroy(tc->dict);
  tc->dict    = NULL;

  free(tc->entries);
  tc->entries = NULL;

  free(tc->sorted);
  tc->sorted  = NULL;

  tc->ntags   = 0;

  /* populate a dictionary with the new tag names and at the same time
   * construct parallel arrays containing dictionary indices and tag counts.
   * the array entries can be re-ordered to sort the data.
   */

  dict = atom_create(); // could suggest sizes using atom_create_tuned()
  if (dict == NULL)
    return error_OOM;

  entries = malloc(ntags * sizeof(*tc->entries));
  if (entries == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  totalcount = 0;

  for (i = 0; i < ntags; i++)
  {
    char   buf[MAXTOKEN];
    atom_t atom;

    /* add a terminator (Font_ScanString returns the 'Illegal control
     * character' error if the string is unterminated). */
    memcpy(buf, tags[i].name, tags[i].length);
    buf[tags[i].length] = '\0';

    err = atom_new(dict,
                   (const unsigned char *) buf,
                   tags[i].length + 1,
                   &atom);
    if (err)
      goto Failure;

    entries[i].atom  = atom;
    entries[i].count = tags[i].count;

    totalcount += tags[i].count;
  }

  /* set up index table (which we use when sorting) */

  sorted = malloc(ntags * sizeof(*sorted));
  if (sorted == NULL)
  {
    err = error_OOM;
    goto Failure;
  }

  for (i = 0; i < ntags; i++)
    sorted[i] = i;


  tc->dict        = dict;

  tc->entries     = entries;
  tc->sorted      = sorted;
  tc->ntags       = ntags;


  calc_scales(tc);

  tc->flags |= tag_cloud_FLAG_NEW_DATA;


  tag_cloud_set_sort(tc, tc->sort_type); /* kick */

  tc->menued_tag_index = -1; // not pleasant doing this here

  tag_cloud_redraw(tc);


  if (tc->flags & tag_cloud_FLAG_TOOLBAR)
  {
    /* set up the stats icon */

    icon_printf(tc->toolbar_w,
                TAG_CLOUD_T_D_INFO,
                message0("tagcloud.stats"),
                ntags,
                totalcount);
  }

  return error_OK;


Failure:

  atom_destroy(dict);
  free(entries);
  free(sorted);

  return err;
}
