/* --------------------------------------------------------------------------
 *    Name: effects.h
 * Purpose: Effects dialogue
 * ----------------------------------------------------------------------- */

#ifndef EFFECTS_H
#define EFFECTS_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

result_t effects_init(void);
void effects_fin(void);
void effects_open(image_t *image);
int effects_available(const image_t *image);

#endif /* EFFECTS_H */
