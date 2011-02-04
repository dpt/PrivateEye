/* --------------------------------------------------------------------------
 *    Name: drawable-bitmap.h
 * Purpose: Generic bitmap methods
 * ----------------------------------------------------------------------- */

#ifndef DRAWABLE_BITMAP
#define DRAWABLE_BITMAP

#include "oslib/os.h"

#include "appengine/graphics/drawable.h"

void bitmap_get_dimensions(drawable_t *drawable, const os_factors *factors, os_box *box);

void drawablebitmap_export_methods(drawable_t *drawable);

#endif /* DRAWABLE_BITMAP */
