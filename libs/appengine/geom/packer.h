/* --------------------------------------------------------------------------
 *    Name: packer.h
 * Purpose: Box packing for layout
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_PACKER_H
#define APPENGINE_PACKER_H

#include "appengine/base/errors.h"

#include "oslib/os.h"

#define T packer_t

typedef struct T T;

T *packer_create(const os_box *dims);
void packer_destroy(T *doomed);

void packer_set_margins(T *packer, const os_box *margins);

typedef enum packer_loc
{
  packer_LOC_TOP_LEFT,
  packer_LOC_TOP_RIGHT,
  packer_LOC_BOTTOM_LEFT,
  packer_LOC_BOTTOM_RIGHT,
  packer_LOC__LIMIT,
}
packer_loc;

/* returns the width of the next available area. */
int packer_next_width(T *packer, packer_loc loc);

/* places an absolutely positioned box 'area'. ignores any margins. */
error packer_place_at(T            *packer,
                      const os_box *area);

/* places a box of dimensions (w,h) in the next free area determined by
 * location 'loc'. */
error packer_place_by(T             *packer,
                      packer_loc     loc,
                      int            w,
                      int            h,
                      const os_box **pos);

typedef enum packer_cleardir
{
  packer_CLEAR_LEFT,
  packer_CLEAR_RIGHT,
  packer_CLEAR_BOTH,
  packer_CLEAR__LIMIT,
}
packer_cleardir;

/* clears up to the next specified boundary. */
error packer_clear(T *packer, packer_cleardir clear);

typedef error (packer_map_fn)(const os_box *area, void *opaque);
/* calls 'fn' for every area known about. */
error packer_map(T *packer, packer_map_fn *fn, void *opaque);

/* returns the union of all areas used. ignores margins. */
const os_box *packer_get_consumed_area(const T *packer);

#undef T

#endif /* APPENGINE_PACKER_H */
