/* --------------------------------------------------------------------------
 *    Name: scale.h
 * Purpose: Scale
 * Version: $Id: scale.h,v 1.13 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_SCALE_H
#define VIEWER_SCALE_H

#include "appengine/graphics/drawable.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

#include "viewer.h"

int scale_for_box(drawable *d, int sw, int sh);

extern dialogue_t *scale;

error viewer_scale_init(void);
void viewer_scale_fin(void);

void scale_set(viewer *viewer, int scale, int redraw);

#endif /* VIEWER_SCALE_H */
