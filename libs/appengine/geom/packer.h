/* --------------------------------------------------------------------------
 *    Name: packer.h
 * Purpose: Box packing for layout
 * Version: $Id: packer.h,v 1.2 2010-01-13 18:41:09 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_PACKER_H
#define APPENGINE_PACKER_H

#include "appengine/base/errors.h"

#include "oslib/os.h"

#define T packer_t

typedef struct T T;

T *packer__create(const os_box *dims);
void packer__destroy(T *doomed);

void packer__set_margins(T *packer, const os_box *margins);

typedef enum packer_loc
{
  packer__LOC_TOP_LEFT,
  packer__LOC_TOP_RIGHT,
  packer__LOC_BOTTOM_LEFT,
  packer__LOC_BOTTOM_RIGHT,
  packer__LOC__LIMIT,
}
packer_loc;

/* returns the width of the next available area. */
int packer__next_width(T *packer, packer_loc loc);

/* places an absolutely positioned box 'area'. ignores any margins. */
error packer__place_at(T            *packer,
                       const os_box *area);

/* places a box of dimensions (w,h) in the next free area determined by
 * location 'loc'. */
error packer__place_by(T             *packer,
                       packer_loc     loc,
                       int            w,
                       int            h,
                       const os_box **pos);

typedef enum packer_clear
{
  packer__CLEAR_LEFT,
  packer__CLEAR_RIGHT,
  packer__CLEAR_BOTH,
  packer__CLEAR__LIMIT,
}
packer_clear;

/* clears up to the next specified boundary. */
error packer__clear(T *packer, packer_clear clear);

typedef error (packer__map_fn)(const os_box *area, void *arg);
/* calls 'fn' for every area known about. */
error packer__map(T *packer, packer__map_fn *fn, void *arg);

/* returns the union of all areas used. ignores margins. */
const os_box *packer__get_consumed_area(const T *packer);

#undef T

#endif /* APPENGINE_PACKER_H */
