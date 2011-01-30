/* --------------------------------------------------------------------------
 *    Name: zones.h
 * Purpose: Zone management header
 * Version: $Id: zones.h,v 1.7 2009-04-28 23:32:37 dpt Exp $
 * ----------------------------------------------------------------------- */

/* A zone is a small monochrome map of which parts of the image have been
 * viewed. The intention is to allow PrivateEye to be able to smoothly move
 * to the next section of unviewed image, e.g. when reading a comic
 * on-screen. */

#ifndef ZONES_H
#define ZONES_H

#include "oslib/wimp.h"

#include "appengine/graphics/image.h"

typedef unsigned int zones;

zones *zones_create(image_t *image);
void zones_destroy(zones *z);
void zones_update(zones *z, wimp_draw *redraw, image_t *image, int scale);
void zones_wherenext(const zones *z);

#endif /* ZONES_H */
