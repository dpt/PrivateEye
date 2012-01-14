/* --------------------------------------------------------------------------
 *    Name: metadata.h
 * Purpose: Metadata windows
 * ----------------------------------------------------------------------- */

#ifndef METADATA_H
#define METADATA_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error metadata_init(void);
void metadata_fin(void);
int metadata_available(const image_t *image);
void metadata_open(image_t  *image,
                   os_colour bgcolour,
                   int       wrapwidth,
                   int       lineheight);

#endif /* METADATA_H */
