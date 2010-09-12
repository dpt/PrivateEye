/* --------------------------------------------------------------------------
 *    Name: zones.h
 * Purpose: Zone management header
 * Version: $Id: zones.h,v 1.7 2009-04-28 23:32:37 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef ZONES_H
#define ZONES_H

#include "oslib/wimp.h"

#include "appengine/graphics/image.h"

typedef unsigned int zones;

zones *zones_create(image *image);
void zones_destroy(zones *z);
void zones_update(zones *z, wimp_draw *redraw, image *image, int scale);
void zones_wherenext(const zones *z);

#endif /* ZONES_H */