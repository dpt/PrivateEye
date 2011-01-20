/* --------------------------------------------------------------------------
 *    Name: hist.h
 * Purpose: Histogram windows
 * Version: $Id: hist.h,v 1.14 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef HIST_H
#define HIST_H

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error hist__init(void);
void hist__fin(void);
int hist__available(const image *image);
void hist__open(image *image, int nbars);

#endif /* HIST_H */
