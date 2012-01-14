/* --------------------------------------------------------------------------
 *    Name: drawable-vector.h
 * Purpose: Generic vector module header
 * ----------------------------------------------------------------------- */

#ifndef DRAWABLE_VECTOR_H
#define DRAWABLE_VECTOR_H

#include "oslib/os.h"

#include "appengine/graphics/drawable.h"

void vector_scaling(drawable_t *drawable,
              const os_factors *factors);

void vector_get_dimensions(drawable_t *drawable,
                     const os_factors *factors,
                           os_box     *box);

#endif /* DRAWABLE_VECTOR_H */
