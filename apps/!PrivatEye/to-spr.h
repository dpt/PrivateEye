/* --------------------------------------------------------------------------
 *    Name: to-spr.h
 * Purpose: Convert to Sprite
 * Version: $Id: to-spr.h,v 1.3 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef TO_SPR_H
#define TO_SPR_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

void to_spr(image *image);
int to_spr__available(const image *image);

#endif /* TO_SPR_H */
