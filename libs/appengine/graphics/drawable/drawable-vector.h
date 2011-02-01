/* --------------------------------------------------------------------------
 *    Name: drawable-vector.h
 * Purpose: Generic vector module header
 * Version: $Id: drawable-vector.h,v 1.1 2009-04-28 23:32:24 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef DRAWABLE_VECTOR_H
#define DRAWABLE_VECTOR_H

#include "oslib/os.h"

#include "appengine/graphics/drawable.h"

void vector_scaling(drawable_t *drawable,
                    const os_factors *factors);

void vector_get_dimensions(drawable_t *drawable,
                           const os_factors *factors,
                           os_box *box);

#endif /* DRAWABLE_VECTOR_H */
