/* --------------------------------------------------------------------------
 *    Name: effects.h
 * Purpose: Effects dialogue
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_EFFECTS_H
#define APPENGINE_EFFECTS_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

typedef struct effectsconfig
{
  int tonemap_stroke_width;
}
effectsconfig_t;

result_t effects_init(void);
void effects_fin(void);
void effects_open(const effectsconfig_t *config,
                  image_t               *image);
int effects_available(const image_t *image);

#endif /* APPENGINE_EFFECTS_H */
