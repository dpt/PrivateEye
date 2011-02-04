/* --------------------------------------------------------------------------
 *    Name: formats.h
 * Purpose: Formats
 * ----------------------------------------------------------------------- */

#ifndef IMAGE_FORMATS_H
#define IMAGE_FORMATS_H

#include "oslib/types.h"

#include "appengine/graphics/image.h"

osbool loader_export_methods(image_choices *choices,
                             image_t       *image,
                             bits           file_type);

#endif /* IMAGE_FORMATS_H */
