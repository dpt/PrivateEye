/* --------------------------------------------------------------------------
 *    Name: effects.h
 * Purpose: Effects dialogue
 * Version: $Id: effects.h,v 1.8 2009-05-20 21:38:18 dpt Exp $
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
