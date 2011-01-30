/* --------------------------------------------------------------------------
 *    Name: tags.h
 * Purpose: Tags window
 * Version: $Id: tags.h,v 1.8 2009-06-11 20:49:35 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef TAGS_H
#define TAGS_H

#include "appengine/app/choices.h"
#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error tags__init(void);
void tags__fin(void);
error tags__open(image_t *image);

error tags__choices_updated(const choices_group *g);

#endif /* TAGS_H */
