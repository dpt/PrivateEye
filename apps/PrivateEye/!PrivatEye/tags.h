/* --------------------------------------------------------------------------
 *    Name: tags.h
 * Purpose: Tags window
 * ----------------------------------------------------------------------- */

#ifndef TAGS_H
#define TAGS_H

#include "appengine/app/choices.h"
#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

result_t tags_substrate_init(void);

result_t tags_init(void);
void tags_fin(void);
result_t tags_open(image_t *image);

result_t tags_choices_updated(const choices_group *g);

#endif /* TAGS_H */
