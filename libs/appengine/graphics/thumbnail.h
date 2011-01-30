/* --------------------------------------------------------------------------
 *    Name: thumbnail.h
 * Purpose: Thumbnail creator
 * Version: $Id: thumbnail.h,v 1.1 2009-09-13 16:21:55 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_THUMBNAILS_H
#define APPENGINE_THUMBNAILS_H

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/drawable.h"
#include "appengine/graphics/image.h"

error thumbnail_create(image_t          *image,
                 const drawable_choices *choices,
                       int               max,
                       osspriteop_area **anchor);
void thumbnail_destroy(osspriteop_area **anchor);

#endif /* APPENGINE_THUMBNAILS_H */
