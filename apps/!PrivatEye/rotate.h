/* --------------------------------------------------------------------------
 *    Name: rotate.h
 * Purpose: Rotation
 * ----------------------------------------------------------------------- */

#ifndef ROTATE_H
#define ROTATE_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

void rotate(image_t *image, int angle, int hflip);

error rotate_init(void);
void rotate_fin(void);
void rotate_open(image_t *image);
int rotate_available(const image_t *image);

#endif /* ROTATE_H */
