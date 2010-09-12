/* --------------------------------------------------------------------------
 *    Name: png.h
 * Purpose: PNG module header
 * Version: $Id: png.h,v 1.1 2009-04-28 23:32:24 dpt Exp $
 * ----------------------------------------------------------------------- */

/* "PNG_H" clashes with something so use IMAGE_PNG_H */
#ifndef IMAGE_PNG_H
#define IMAGE_PNG_H

#include "appengine/graphics/image.h"

extern void png_export_methods(image_choices *choices, image *image);

#endif /* IMAGE_PNG_H */
