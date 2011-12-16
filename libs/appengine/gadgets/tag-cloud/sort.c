/* --------------------------------------------------------------------------
 *    Name: sort.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

/* TODO
 *
 * The sorting highlights first stuff can likely be implemented more
 * efficiently by first sorting the highlights into two bins then sorting
 * within each.
 */

#include <stdlib.h>
#include <string.h>

#include "appengine/types.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/atom.h"

#include "appengine/gadgets/tag-cloud.h"

#include "iconnames.h"
#include "impl.h"

static tag_cloud *sort_tc;

static int sort_by_name(const void *va, const void *vb)
{
  const int  *a = va;
  const int  *b = vb;
  const char *sa;
  const char *sb;

  sa = (const char *) atom_get(sort_tc->dict, *a, NULL);
  sb = (const char *) atom_get(sort_tc->dict, *b, NULL);

  if (sa == sb)
    return 0;

  return strcasecmp(sa, sb);
}

/* This sorts high counts first. */
static int sort_by_count(const void *va, const void *vb)
{
  const int *a      = va;
  const int *b      = vb;
  int        counta = sort_tc->entries[*a].count;
  int        countb = sort_tc->entries[*b].count;

       if (counta > countb) return -1;
  else if (counta < countb) return  1;

  return *a - *b; /* compare indices to keep sort stable */
}

/* sort_by_name with highlights first */
static int sort_by_name_hi(const void *va, const void *vb)
{
  const int *a = va;
  const int *b = vb;
  int        ia;
  int        ib;
  int        ha;
  int        hb;

  ia = *a;
  ib = *b;

  ha = tag_cloud__is_highlighted(sort_tc, ia);
  hb = tag_cloud__is_highlighted(sort_tc, ib);

  /* we want highlights to sort sooner, so if it's highlighted then return a
   * _lower_ value */

  if (ha > hb)
    return -1;
  else if (ha < hb)
    return 1;

  return sort_by_name(va, vb);
}

/* sort_by_count with highlights first */
static int sort_by_count_hi(const void *va, const void *vb)
{
  const int *a = va;
  const int *b = vb;
  int        ia;
  int        ib;
  int        ha;
  int        hb;

  ia = *a;
  ib = *b;

  ha = tag_cloud__is_highlighted(sort_tc, ia);
  hb = tag_cloud__is_highlighted(sort_tc, ib);

  /* we want highlights to sort sooner, so if it's highlighted then return a
   * _lower_ value */

  if (ha > hb)
    return -1;
  else if (ha < hb)
    return 1;

  return sort_by_count(va, vb);
}

typedef int (*compar)(const void *, const void *);

static void sort(tag_cloud *tc)
{
  static const compar fn[2][2] =
  {
    { sort_by_name,    sort_by_count    },
    { sort_by_name_hi, sort_by_count_hi },
  };

  int sort_type;
  int order_type;

  sort_type  = tag_cloud__get_sort(tc);
  order_type = tag_cloud__get_order(tc);

  /* avoid sorting whereever possible */

  if (sort_type  == tc->sort.last_sort_type &&
      order_type == tc->sort.last_order_type &&
      (tc->flags & tag_cloud__FLAG_NEW_DATA) == 0 &&
      (tc->flags & tag_cloud__FLAG_NEW_HIGHLIGHTS) == 0)
    return; /* already sorted ### check */

  /* qsort doesn't have any way to pass an arg through to the compare
   * function, so here we're forced to store 'tc' in a static.
   */
  sort_tc = tc;

  qsort(tc->sorted, tc->e_used, sizeof(*tc->sorted),
        fn[order_type][sort_type]);

  tc->flags |= tag_cloud__FLAG_NEW_SORT;

  tc->sort.last_sort_type  = sort_type;
  tc->sort.last_order_type = order_type;

  tag_cloud__schedule_redraw(tc);
}

/* ----------------------------------------------------------------------- */

static void tag_cloud__kick_sort_icon(tag_cloud *tc)
{
  if (tc->flags & tag_cloud__FLAG_TOOLBAR)
    /* toolbar icons must match the order of sort_type */
    icon_set_radio(tc->toolbar_w, TAG_CLOUD_T_B_SORTALPHA + tc->sort_type);
}

void tag_cloud__set_sort(tag_cloud *tc, int sort_type)
{
  if ((unsigned int) sort_type >= tag_cloud__SORT_TYPE__LIMIT)
    sort_type = 0;

  tc->sort_type = sort_type;

  sort(tc);

  tag_cloud__kick_sort_icon(tc);
}

int tag_cloud__get_sort(tag_cloud *tc)
{
  return tc->sort_type;
}

/* ----------------------------------------------------------------------- */

static void tag_cloud__kick_order_icon(tag_cloud *tc)
{
  if (tc->flags & tag_cloud__FLAG_TOOLBAR)
    icon_set_selected(tc->toolbar_w, TAG_CLOUD_T_O_SELFIRST,
                      tc->flags & tag_cloud__FLAG_SORT_SEL_FIRST);
}

void tag_cloud__set_order(tag_cloud *tc, int order_type)
{
  if ((unsigned int) order_type >= tag_cloud__ORDER__LIMIT)
    order_type = 0;

  if (order_type == tag_cloud__ORDER_SELECTED_FIRST)
    tc->flags |= tag_cloud__FLAG_SORT_SEL_FIRST;
  else
    tc->flags &= ~tag_cloud__FLAG_SORT_SEL_FIRST;

  sort(tc);

  tag_cloud__kick_order_icon(tc);
}

void tag_cloud__toggle_order(tag_cloud *tc)
{
  tc->flags ^= tag_cloud__FLAG_SORT_SEL_FIRST;

  sort(tc);

  tag_cloud__kick_order_icon(tc);
}

int tag_cloud__get_order(tag_cloud *tc)
{
  if (tc->flags & tag_cloud__FLAG_SORT_SEL_FIRST)
    return tag_cloud__ORDER_SELECTED_FIRST;
  else
    return 0;
}
