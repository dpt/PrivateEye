/* --------------------------------------------------------------------------
 *    Name: png.h
 * Purpose: PNG module header
 * ----------------------------------------------------------------------- */

/* "PNG_H" clashes with something so use IMAGE_PNG_H */
#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#include "appengine/graphics/image.h"

extern void png_export_methods(image_choices *choices, image_t *image);

#endif /* IMAGE_PNG_H */
