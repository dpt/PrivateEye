/* --------------------------------------------------------------------------
 *    Name: rotate.h
 * Purpose: Rotation
 * Version: $Id: rotate.h,v 1.14 2009-09-08 23:10:40 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef ROTATE_H
#define ROTATE_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

void rotate(image *image, int angle, int hflip);

error rotate__init(void);
void rotate__fin(void);
void rotate__open(image *image);
int rotate__available(const image *image);

#endif /* ROTATE_H */
