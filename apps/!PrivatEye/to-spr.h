/* --------------------------------------------------------------------------
 *    Name: to-spr.h
 * Purpose: Convert to Sprite
 * ----------------------------------------------------------------------- */

#ifndef TO_SPR_H
#define TO_SPR_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

void to_spr(image_t *image);
int to_spr__available(const image_t *image);

#endif /* TO_SPR_H */
