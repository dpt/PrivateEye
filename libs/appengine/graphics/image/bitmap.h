/* --------------------------------------------------------------------------
 *    Name: bitmap.h
 * Purpose: Generic bitmap module header
 * Version: $Id: bitmap.h,v 1.1 2009-04-28 23:32:24 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef IMAGE_BITMAP_H
#define IMAGE_BITMAP_H

#include "appengine/graphics/image.h"

int bitmap_save(image_choices *choices, image *image, const char *file_name);
int bitmap_unload(image *image);
int bitmap_histogram(image *image);
int bitmap_rotate(image_choices *choices, image *image, int angle);

#endif /* IMAGE_BITMAP_H */
