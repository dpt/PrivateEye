/* --------------------------------------------------------------------------
 *    Name: formats.h
 * Purpose: Formats
 * Version: $Id: formats.h,v 1.1 2009-04-28 23:32:24 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef IMAGE_FORMATS_H
#define IMAGE_FORMATS_H

#include "oslib/types.h"

#include "appengine/graphics/image.h"

osbool loader_export_methods(image_choices *choices,
                             image         *image,
                             bits           file_type);

#endif /* IMAGE_FORMATS_H */
