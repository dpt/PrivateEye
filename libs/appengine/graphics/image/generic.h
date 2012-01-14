/* --------------------------------------------------------------------------
 *    Name: generic.h
 * Purpose: Generic module header
 * ----------------------------------------------------------------------- */

#ifndef IMAGE_GENERIC_H
#define IMAGE_GENERIC_H

#include "appengine/graphics/image.h"

int generic_save(image_choices *choices,
                 image_t       *image,
                 const char    *file_name);

#endif /* IMAGE_GENERIC_H */
