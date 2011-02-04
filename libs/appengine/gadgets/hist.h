/* --------------------------------------------------------------------------
 *    Name: hist.h
 * Purpose: Histogram windows
 * ----------------------------------------------------------------------- */

#ifndef HIST_H
#define HIST_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error hist__init(void);
void hist__fin(void);
int hist__available(const image_t *image);
void hist__open(image_t *image, int nbars);

#endif /* HIST_H */
