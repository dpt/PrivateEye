/* --------------------------------------------------------------------------
 *    Name: hist.h
 * Purpose: Histogram windows
 * ----------------------------------------------------------------------- */

#ifndef HIST_H
#define HIST_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

result_t hist_init(void);
void hist_fin(void);
int hist_available(const image_t *image);
void hist_open(image_t *image, int nbars);

#endif /* HIST_H */
