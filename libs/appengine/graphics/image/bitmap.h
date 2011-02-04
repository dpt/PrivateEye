/* --------------------------------------------------------------------------
 *    Name: bitmap.h
 * Purpose: Generic bitmap module header
 * ----------------------------------------------------------------------- */

#ifndef IMAGE_BITMAP_H
#define IMAGE_BITMAP_H

#include "appengine/graphics/image.h"

int bitmap_save(image_choices *choices, image_t *image, const char *file_name);
int bitmap_unload(image_t *image);
int bitmap_histogram(image_t *image);
int bitmap_rotate(image_choices *choices, image_t *image, int angle);

#endif /* IMAGE_BITMAP_H */
