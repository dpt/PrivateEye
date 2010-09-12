/* --------------------------------------------------------------------------
 *    Name: convolve.h
 * Purpose: Convolves images
 * Version: $Id: convolve.h,v 1.2 2009-05-18 22:07:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef CONVOLVE_H
#define CONVOLVE_H

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"

typedef struct convolve_lut convolve_lut;

error convolve_init(const float   *kernel,
                    int            n,
                    convolve_lut **lut);

void convolve_destroy(convolve_lut *lut);

error convolve_sprite(const convolve_lut      *lut,
                      const osspriteop_area   *area,
                      const osspriteop_header *src,
                      osspriteop_header       *dst);

#endif /* CONVOLVE_H */
