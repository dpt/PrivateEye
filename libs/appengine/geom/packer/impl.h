/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Box packing for layout
 * ----------------------------------------------------------------------- */

#ifndef IMPL_H
#define IMPL_H

#include "oslib/os.h"

#include "appengine/geom/packer.h"

typedef enum packer_sort
{
  packer__SORT_TOP_LEFT,
  packer__SORT_TOP_RIGHT,
  packer__SORT_BOTTOM_LEFT,
  packer__SORT_BOTTOM_RIGHT,
  packer__SORT__LIMIT,
}
packer_sort;

struct packer_t
{
  os_box       *areas;
  int           allocedareas;
  int           usedareas;

  os_box        dims;          /* page size */

  os_box        margins;       /* page size minus margins (but not the
                                  margins themselves) */

  int           nextindex;
  os_box        nextarea;      /* area returned by packer__next */

  os_box        placed_area;   /* last packer__place_by result */

  packer_sort   order;         /* order to which we have sorted */
  int           sorted;        // bool

  os_box        consumed_area; /* total consumed area */
};

#endif /* IMPL_H */
