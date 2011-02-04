/* --------------------------------------------------------------------------
 *    Name: rotate.h
 * Purpose: Rotation
 * ----------------------------------------------------------------------- */

#ifndef ROTATE_H
#define ROTATE_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

void rotate(image_t *image, int angle, int hflip);

error rotate__init(void);
void rotate__fin(void);
void rotate__open(image_t *image);
int rotate__available(const image_t *image);

#endif /* ROTATE_H */
