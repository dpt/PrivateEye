/* --------------------------------------------------------------------------
 *    Name: metadata.h
 * Purpose: Metadata windows
 * ----------------------------------------------------------------------- */

#ifndef METADATA_H
#define METADATA_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error metadata__init(void);
void metadata__fin(void);
int metadata__available(const image_t *image);
void metadata__open(image_t *image,
                    os_colour bgcolour, int wrapwidth, int lineheight);

#endif /* METADATA_H */
