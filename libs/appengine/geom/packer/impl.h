/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Box packing for layout
 * ----------------------------------------------------------------------- */

#ifndef IMPL_H
#define IMPL_H

#include "oslib/os.h"

#include "appengine/geom/packer.h"

typedef enum packer_sortdir
{
  packer_SORT_TOP_LEFT,
  packer_SORT_TOP_RIGHT,
  packer_SORT_BOTTOM_LEFT,
  packer_SORT_BOTTOM_RIGHT,
  packer_SORT__LIMIT,
}
packer_sortdir;

struct packer_t
{
  os_box        *areas;
  int            allocedareas;
  int            usedareas;

  os_box         dims;          /* page size */

  os_box         margins;       /* page size minus margins (but not the
                                   margins themselves) */

  int            nextindex;
  os_box         nextarea;      /* area returned by packer_next */

  os_box         placed_area;   /* last packer_place_by result */

  packer_sortdir order;         /* order to which we have sorted */
  int            sorted;        // bool

  os_box         consumed_area; /* total consumed area */
};

#endif /* IMPL_H */
