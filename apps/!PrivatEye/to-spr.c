/* --------------------------------------------------------------------------
 *    Name: to-spr.c
 * Purpose: Convert to Sprite
 * Version: $Id: to-spr.c,v 1.3 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/graphics/image.h"
#include "appengine/base/os.h"

#include "to-spr.h"

void to_spr(image_t *image)
{
  if (!to_spr__available(image))
  {
    beep();
    return;
  }

  image->methods.to_spr(image);
}

int to_spr__available(const image_t *image)
{
  return image &&
         !image_is_editing(image) &&
         image->flags & image_FLAG_CAN_SPR;
}
