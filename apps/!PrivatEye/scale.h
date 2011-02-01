/* --------------------------------------------------------------------------
 *    Name: scale.h
 * Purpose: Viewer scale dialogue
 * Version: $Id: scale.h,v 1.13 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_SCALE_H
#define VIEWER_SCALE_H

#include "appengine/graphics/drawable.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

#include "viewer.h"

int viewer_scale_for_box(drawable_t *d, int sw, int sh);

extern dialogue_t *viewer_scale;

error viewer_scale_init(void);
void viewer_scale_fin(void);

void viewer_scale_set(viewer_t *viewer, int scale, int redraw);

#endif /* VIEWER_SCALE_H */
