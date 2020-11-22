/* --------------------------------------------------------------------------
 *    Name: scale.h
 * Purpose: Viewer scale dialogue handler
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_SCALEDLG_H
#define VIEWER_SCALEDLG_H

#include "appengine/graphics/drawable.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

#include "viewer.h"

int viewer_scale_for_box(drawable_t *d, int sw, int sh);

extern dialogue_t *viewer_scaledlg;

error viewer_scaledlg_init(void);
void viewer_scaledlg_fin(void);

void viewer_scaledlg_set(viewer_t *viewer, int scale, int redraw);

#endif /* VIEWER_SCALEDLG_H */
