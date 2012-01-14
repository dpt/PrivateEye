/* --------------------------------------------------------------------------
 *    Name: to-spr.c
 * Purpose: Convert to Sprite
 * ----------------------------------------------------------------------- */

#include "appengine/base/os.h"
#include "appengine/graphics/image.h"

#include "to-spr.h"

void to_spr(image_t *image)
{
  if (!to_spr_available(image))
  {
    beep();
    return;
  }

  image->methods.to_spr(image);
}

int to_spr_available(const image_t *image)
{
  return image &&
        !image_is_editing(image) &&
         image->flags & image_FLAG_CAN_SPR;
}
