/* --------------------------------------------------------------------------
 *    Name: convolve.h
 * Purpose: Convolves images
 * ----------------------------------------------------------------------- */

#ifndef CONVOLVE_H
#define CONVOLVE_H

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"

typedef struct convolve_lut convolve_lut;

result_t convolve_init(const float   *kernel,
                       int            n,
                       convolve_lut **lut);

void convolve_destroy(convolve_lut *lut);

result_t convolve_sprite(const convolve_lut      *lut,
                         const osspriteop_area   *area,
                         const osspriteop_header *src,
                         osspriteop_header       *dst);

#endif /* CONVOLVE_H */
