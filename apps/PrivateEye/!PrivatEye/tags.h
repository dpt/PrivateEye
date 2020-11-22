/* --------------------------------------------------------------------------
 *    Name: tags.h
 * Purpose: Tags window
 * ----------------------------------------------------------------------- */

#ifndef TAGS_H
#define TAGS_H

#include "appengine/app/choices.h"
#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error tags_substrate_init(void);

error tags_init(void);
void tags_fin(void);
error tags_open(image_t *image);

error tags_choices_updated(const choices_group *g);

#endif /* TAGS_H */
