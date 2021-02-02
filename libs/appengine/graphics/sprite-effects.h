/* --------------------------------------------------------------------------
 *    Name: sprite-effects.h
 * Purpose:
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SPRITE_EFFECTS_H
#define APPENGINE_SPRITE_EFFECTS_H

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/tonemap.h"

typedef int blender_lut;

typedef struct blender
{
  int       *dst;

  const int *src;
  const int *deg; /* degenerate */

  const int *src_to_use;
  const int *deg_to_use;

  int        width, height;
  int        bpp;

  void     (*make_lut)(int             alpha,  /* 16.16  */
                       int             offset, /* 0..255 */
                       struct blender *args);
  void     (*blend)(struct blender *args);

  blender_lut lutA[256];
  blender_lut lutB[256];
}
blender;

result_t blender_create(blender *b, osspriteop_area *area);

typedef enum effects_blur_method
{
  effects_blur_BOX,
  effects_blur_GAUSSIAN,
}
effects_blur_method;

result_t effects_blur_apply(osspriteop_area    *area,
                            osspriteop_header  *src,
                            osspriteop_header  *dst,
                            effects_blur_method method,
                            int                 amount);

result_t effects_clear_apply(osspriteop_area   *area,
                             osspriteop_header *src,
                             osspriteop_header *dst,
                             os_colour          colour);

result_t effects_equalise_apply(osspriteop_area   *area,
                                osspriteop_header *src,
                                osspriteop_header *dst);

result_t effects_expand_apply(osspriteop_area   *area,
                              osspriteop_header *src,
                              osspriteop_header *dst,
                              unsigned int       threshold);

result_t effects_grey_apply(osspriteop_area   *area,
                            osspriteop_header *src,
                            osspriteop_header *dst);

result_t effects_sharpen_apply(osspriteop_area   *area,
                               osspriteop_header *src,
                               osspriteop_header *dst,
                               int                amount);

result_t effects_tonemap_apply(osspriteop_area   *area,
                               osspriteop_header *src,
                               osspriteop_header *dst,
                               tonemap           *map);

#endif /* APPENGINE_SPRITE_EFFECTS_H */
