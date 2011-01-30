/* --------------------------------------------------------------------------
 *    Name: metadata.h
 * Purpose: Metadata
 * Version: $Id: metadata.h,v 1.3 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef METADATA_H
#define METADATA_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error metadata__init(void);
void metadata__fin(void);
int metadata__available(const image_t *image);
void metadata__open(image_t *image);

#endif /* METADATA_H */
