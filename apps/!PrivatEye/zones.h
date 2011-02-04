/* --------------------------------------------------------------------------
 *    Name: zones.h
 * Purpose: Zone management header
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
