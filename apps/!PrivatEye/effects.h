/* --------------------------------------------------------------------------
 *    Name: effects.h
 * Purpose: Effects dialogue
 * ----------------------------------------------------------------------- */

#ifndef EFFECTS_H
#define EFFECTS_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error effects__init(void);
void effects__fin(void);
void effects__open(image_t *image);
int effects__available(const image_t *image);

#endif /* EFFECTS_H */
