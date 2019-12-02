/* --------------------------------------------------------------------------
 *    Name: packer.c
 * Purpose: Box packing for layout
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/os.h"

#include "appengine/base/errors.h"
#include "appengine/geom/box.h"

#include "appengine/geom/packer.h"

#include "impl.h"

#ifndef NDEBUG
#define debugf(_p) printf _p
#else
#define debugf(_p)
#endif

#define INITIALAREAS 8

packer_t *packer_create(const os_box *dims)
{
  packer_t *p;

  p = malloc(sizeof(*p));
  if (p == NULL)
    return NULL;

  p->areas = malloc(sizeof(*p->areas) * INITIALAREAS);
  if (p->areas == NULL)
  {
    free(p);
    return NULL;
  }

  p->allocedareas     = INITIALAREAS;

  p->areas[0]         = *dims;
  p->usedareas        = 1;

  p->dims             = *dims;
  p->margins          = *dims;

  p->order            = packer_SORT_TOP_LEFT; /* any will do */
  p->sorted           = 1;

  p->consumed_area.x0 = INT_MAX;
  p->consumed_area.y0 = INT_MAX;
  p->consumed_area.x1 = INT_MIN;
  p->consumed_area.y1 = INT_MIN;

  return p;
}

void packer_destroy(packer_t *doomed)
{
  if (doomed == NULL)
    return;

  free(doomed->areas);
  free(doomed);
}

/* ----------------------------------------------------------------------- */

void packer_set_margins(packer_t *packer, const os_box *margins)
{
  packer->margins.x0 = packer->dims.x0 + margins->x0; /* left */
  packer->margins.y0 = packer->dims.y0 + margins->y0; /* bottom */
  packer->margins.x1 = packer->dims.x1 - margins->x1; /* right */
  packer->margins.y1 = packer->dims.y1 - margins->y1; /* top */

  if (box_is_empty(&packer->margins))
  {
    /* keep valid */

    debugf(("is empty\n"));

    packer->margins.x1 = packer->margins.x0 + 1;
    packer->margins.y1 = packer->margins.y0 + 1;
  }

  debugf(("with margins: %d %d %d %d\n",
          packer->margins.x0,
          packer->margins.y0,
          packer->margins.x1,
          packer->margins.y1));
}

/* ----------------------------------------------------------------------- */

static error add_area(packer_t *packer, const os_box *area)
{
  int i;

  debugf(("add_area: %d %d %d %d\n",
          area->x0,
          area->y0,
          area->x1,
          area->y1));

  /* is the new 'area' entirely contained by an existing area? */

  for (i = 0; i < packer->usedareas; i++)
  {
    if (box_is_empty(&packer->areas[i]) ||
       !box_contains_box(area, &packer->areas[i]))
      continue;

    debugf(("add_area: contained by: %d <%d %d %d %d>\n",
            i,
            packer->areas[i].x0,
            packer->areas[i].y0,
            packer->areas[i].x1,
            packer->areas[i].y1));

    return error_OK; /* entirely contained */
  }

  if (packer->usedareas + 1 > packer->allocedareas)
  {
    int     n;
    os_box *areas;

    n = packer->allocedareas * 2;

    areas = realloc(packer->areas, n * sizeof(*areas));
    if (areas == NULL)
      return error_OOM;

    packer->areas        = areas;
    packer->allocedareas = n;

    debugf(("add_area: growing list to %d\n", n));
  }

  packer->areas[packer->usedareas++] = *area;

  packer->sorted = 0;

  return error_OK;
}

static error remove_area(packer_t *packer, const os_box *area)
{
  error err;
  int   n;
  int   i;

  debugf(("remove_area: %d %d %d %d\n",
          area->x0,
          area->y0,
          area->x1,
          area->y1));

  /* now split up any free areas which overlap with 'area' */

  n = packer->usedareas; /* use a stored copy as we'll be changing
                          * packer->usedareas inside the loop */

  for (i = 0; i < n; i++)
  {
    os_box saved;
    os_box subarea;

    saved = packer->areas[i];

    if (box_is_empty(&saved) || !box_intersects(&saved, area))
      continue;

    /* invalidate the area */

    packer->areas[i].x0 = 0;
    packer->areas[i].y0 = 0;
    packer->areas[i].x1 = 0;
    packer->areas[i].y1 = 0;

    packer->sorted = 0;

    /* store all remaining available areas */

    if (saved.x0 < area->x0)
    {
      /* available space remains on the left */
      subarea    = saved;
      subarea.x1 = area->x0;
      err = add_area(packer, &subarea);
      if (err)
        goto failure;
    }

    if (saved.x1 > area->x1)
    {
      /* available space remains on the right */
      subarea    = saved;
      subarea.x0 = area->x1;
      err = add_area(packer, &subarea);
      if (err)
        goto failure;
    }

    if (saved.y0 < area->y0)
    {
      /* available space remains at the bottom */
      subarea    = saved;
      subarea.y1 = area->y0;
      err = add_area(packer, &subarea);
      if (err)
        goto failure;
    }

    if (saved.y1 > area->y1)
    {
      /* available space remains at the top */
      subarea    = saved;
      subarea.y0 = area->y1;
      err = add_area(packer, &subarea);
      if (err)
        goto failure;
    }
  }

  box_union(&packer->consumed_area, area, &packer->consumed_area);

  return error_OK;


failure:

  return err;
}

/* ----------------------------------------------------------------------- */

#define COMPARE_AREAS(name,ta,tb,tc)                            \
static int compare_areas_##name(const void *va, const void *vb) \
{                                                               \
  const os_box *a = va, *b = vb;                                \
  int           emptya, emptyb;                                 \
  int           v;                                              \
                                                                \
  emptya = box_is_empty(a);                                     \
  emptyb = box_is_empty(b);                                     \
                                                                \
  /* place invalid boxes towards the end of the list */         \
                                                                \
  if (emptya && emptyb)                                         \
    return 0;                                                   \
  else if (emptya)                                              \
    return 1;                                                   \
  else if (emptyb)                                              \
    return -1;                                                  \
                                                                \
  v = ta; if (v) return v;                                      \
  v = tb; if (v) return v;                                      \
  v = tc;        return v;                                      \
}

COMPARE_AREAS(top_left,     b->y1 - a->y1, a->x0 - b->x0, b->x1 - a->x1)
COMPARE_AREAS(top_right,    b->y1 - a->y1, b->x1 - a->x1, a->x0 - b->x0)
COMPARE_AREAS(bottom_left,  a->y0 - b->y0, a->x0 - b->x0, b->x1 - a->x1)
COMPARE_AREAS(bottom_right, a->y0 - b->y0, b->x1 - a->x1, a->x0 - b->x0)

static void packer_sort(packer_t *packer, packer_sortdir order)
{
  os_box       *areas;
  int           usedareas;
  const os_box *end;
  os_box       *b;

  assert(order < packer_SORT__LIMIT);

  if (packer->sorted && packer->order == order)
    return;

  areas     = packer->areas;
  usedareas = packer->usedareas;

  if (usedareas > 1)
  {
    int (*compare)(const void *, const void *);

    debugf(("packer_sort: sorting areas to %d\n", order));

    switch (order)
    {
    default:
    case packer_SORT_TOP_LEFT:
      compare = compare_areas_top_left;
      break;
    case packer_SORT_TOP_RIGHT:
      compare = compare_areas_top_right;
      break;
    case packer_SORT_BOTTOM_LEFT:
      compare = compare_areas_bottom_left;
      break;
    case packer_SORT_BOTTOM_RIGHT:
      compare = compare_areas_bottom_right;
      break;
    }

    qsort(areas, usedareas, sizeof(*areas), compare);
  }

  debugf(("packer_sort: trimming\n"));

  end = areas + usedareas;

  for (b = areas; b < end; b++)
  {
    int n;

    /* trim away any invalid areas which will have been sorted to the end
     * of the list */

    debugf(("%d %d %d %d\n", b->x0, b->y0, b->x1, b->y1));

    if (!box_is_empty(b))
      continue;

    n = b - areas;

    debugf(("trimming to %d long\n", n));
    packer->usedareas = n;
    break;
  }

  packer->sorted = 1;
  packer->order  = order;
}

/* ----------------------------------------------------------------------- */

static const os_box *packer_next(packer_t *packer)
{
  do
  {
    if (packer->nextindex >= packer->usedareas)
    {
      debugf(("next: none left\n"));
      return NULL; /* no more areas */
    }

    box_intersection(&packer->areas[packer->nextindex++],
                     &packer->margins,
                     &packer->nextarea);
  }
  while (box_is_empty(&packer->nextarea));

  debugf(("next: %d %d %d %d\n",
          packer->nextarea.x0,
          packer->nextarea.y0,
          packer->nextarea.x1,
          packer->nextarea.y1));

  return &packer->nextarea;
}

static const os_box *packer_start(packer_t *packer, packer_sortdir order)
{
  packer_sort(packer, order);

  packer->nextindex = 0;

  debugf(("start: list is %d long\n", packer->usedareas));

  return packer_next(packer);
}

/* ----------------------------------------------------------------------- */

int packer_next_width(packer_t *packer, packer_loc loc)
{
  const os_box *b;

  b = packer_start(packer, (packer_sortdir) loc);
  if (!b)
    return 0;

  return b->x1 - b->x0;
}

/* ----------------------------------------------------------------------- */

error packer_place_at(packer_t *packer, const os_box *area)
{
  os_box b;

  /* subtract the margins */

  box_intersection(&packer->margins, area, &b);

  if (box_is_empty(&b))
    return error_PACKER_EMPTY;

  return remove_area(packer, &b);
}

error packer_place_by(packer_t  *packer,
                      packer_loc loc,
                      int        w,
                      int        h,
                const os_box   **pos)
{
  error         err;
  const os_box *b;

  if (pos)
    *pos = NULL;

  if (w == 0 || h == 0)
    return error_PACKER_EMPTY;

  for (b = packer_start(packer, (packer_sortdir) loc);
       b;
       b = packer_next(packer))
  {
    if (box_could_hold(b, w, h))
    {
      debugf(("packer_place_by: fits\n"));
      break;
    }
  }

  if (!b)
  {
    debugf(("packer_place_by: didn't fit\n"));
    return error_PACKER_DIDNT_FIT;
  }

  switch (loc)
  {
  case packer_LOC_TOP_LEFT:
  case packer_LOC_BOTTOM_LEFT:
    packer->placed_area.x0 = b->x0;
    packer->placed_area.x1 = b->x0 + w;
    break;

  case packer_LOC_TOP_RIGHT:
  case packer_LOC_BOTTOM_RIGHT:
    packer->placed_area.x0 = b->x1 - w;
    packer->placed_area.x1 = b->x1;
    break;
  }

  switch (loc)
  {
  case packer_LOC_TOP_LEFT:
  case packer_LOC_TOP_RIGHT:
    packer->placed_area.y0 = b->y1 - h;
    packer->placed_area.y1 = b->y1;
    break;

  case packer_LOC_BOTTOM_LEFT:
  case packer_LOC_BOTTOM_RIGHT:
    packer->placed_area.y0 = b->y0;
    packer->placed_area.y1 = b->y0 + h;
    break;
  }

  err = remove_area(packer, &packer->placed_area);
  if (err)
    return err;

  if (pos)
    *pos = &packer->placed_area;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error packer_clear(packer_t *packer, packer_cleardir clear)
{
  error         err;
  int           left, right;
  const os_box *b;
  os_box        clearbox;

  debugf(("packer_clear\n"));

  switch (clear)
  {
  default:
  case packer_CLEAR_LEFT:
    left  = packer->margins.x0;
    right = INT_MIN;
    break;
  case packer_CLEAR_RIGHT:
    left  = INT_MAX;
    right = packer->margins.x1;
    break;
  case packer_CLEAR_BOTH:
    left  = packer->margins.x0;
    right = packer->margins.x1;
    break;
  }

  for (b = packer_start(packer, packer_SORT_TOP_LEFT);
       b;
       b = packer_next(packer))
  {
    /* find an area which touches the required edge(s) */

    if (b->x0 <= left && b->x1 >= right)
      break; /* found the edge(s) */
  }

  /* clear from top of found box upwards to top of doc */

  debugf(("packer_clear: invalidate area\n"));

  clearbox = packer->margins;

  /* if no edges were found then clear the whole area */

  if (b)
    clearbox.y0 = b->y1;

  err = remove_area(packer, &clearbox);
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error packer_map(packer_t *packer, packer_map_fn *fn, void *opaque)
{
  error         err;
  const os_box *b;

  for (b = packer_start(packer, packer->order);
       b;
       b = packer_next(packer))
  {
    err = fn(b, opaque);
    if (err)
      return err;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

const os_box *packer_get_consumed_area(const packer_t *packer)
{
  return &packer->consumed_area;
}
