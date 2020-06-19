/* --------------------------------------------------------------------------
 *    Name: thumbnail.c
 * Purpose: Thumbnail creator
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/hourglass.h"

#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"
#include "appengine/graphics/drawable.h"

#include "appengine/graphics/thumbnail.h"

error thumbnail_create(image_t          *image,
                 const drawable_choices *choices,
                       int               max,
                       osspriteop_area **anchor)
{
  error            err;
  osspriteop_area *area;
  int              w,h;
  int              log2bpp;
  size_t           sprimgbytes;
  os_mode          mode;
  int              c0,c1,c2,c3;
  drawable_t      *drawable;
  os_factors       factors;
  wimp_draw        draw;

  hourglass_on();

  if (image->flags & image_FLAG_VECTOR)
  {
    err = error_THUMBNAIL_UNSUPP_FUNC;
    goto Failure;
  }

  /* work out new dimensions */

  /* 484 x 644
      80 x 106.446281
      ChangeFSI uses 107 high */

  w = max;
  h = image->display.dims.bm.height * w / image->display.dims.bm.width;

  if (h > max)
  {
    h = max;
    w = image->display.dims.bm.width * h / image->display.dims.bm.height;
  }

  /* work out sprite area size */
  /* will this work for vector images? probably not */

  read_current_mode_vars(NULL, NULL, &log2bpp);

  sprimgbytes = sprite_size(w, h, log2bpp, FALSE);

  /* allocate an area */

  if (flex_alloc((flex_ptr) &area, sprimgbytes) == 0)
  {
    err = error_OOM;
    goto Failure;
  }

  /* init area */

  area->size  = sprimgbytes;
  area->first = 16;
  osspriteop_clear_sprites(osspriteop_USER_AREA, area);

  /* create sprite */

  mode = (os_mode) (((log2bpp + 1) << osspriteop_TYPE_SHIFT) |
                               (90 << osspriteop_YRES_SHIFT) |
                               (90 << osspriteop_XRES_SHIFT) |
                                      osspriteop_NEW_STYLE);

  osspriteop_create_sprite(osspriteop_USER_AREA,
                           area,
                           "thumbnail",
                           0, /* no palette */
                           w, h,
                           mode);

  /* redirect to sprite */

  osspriteop_switch_output_to_sprite(osspriteop_USER_AREA,
                                     area,
                     (osspriteop_id) "thumbnail",
                                     0,
                                     &c0, &c1, &c2, &c3);

  cache_mode_vars();

  /* set up drawing stuff */

  err = drawable_create(image, &drawable);
  if (err)
    goto Failure;

  /* calculate pixtrans / colourtrans */
  /* since it's redirected, the current mode settings should pick it up */

  if (drawable->methods.update_colours)
    drawable->methods.update_colours(drawable);

  os_factors_from_ratio(&factors, w, image->display.dims.bm.width);

  drawable->methods.update_scaling(drawable, &factors);

  /* call draw entry point */
  draw.w = NONE;

  /* FIXME: This works for now, but is probably quite fragile. */

  /* draw->box  (most stuff does x0-xscroll,y1-yscroll)
     draw->clip (for artworks) */

  draw.box.x0  = 0;
  draw.box.y0  = 0;
  draw.box.x1  = 0;
  draw.box.y1  = 0;
  draw.xscroll = 0;
  draw.yscroll = 0;
  draw.clip.x0 = 0;
  draw.clip.y0 = 0;
  draw.clip.x1 = 0;
  draw.clip.y1 = 0;

  drawable->methods.redraw(choices, &draw, drawable, 0, 0);

  /* redirect back */

  osspriteop_unswitch_output(c0, c1, c2, c3);

  cache_mode_vars();


  drawable_destroy(drawable);


  hourglass_off();

  flex_reanchor((flex_ptr) anchor, (flex_ptr) &area);

  return error_OK;

Failure:

  hourglass_off();

  return err;
}

void thumbnail_destroy(osspriteop_area **anchor)
{
  flex_free((flex_ptr) anchor);
}
