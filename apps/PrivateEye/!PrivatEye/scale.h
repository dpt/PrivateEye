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

int drawable_best_fit_for_size(drawable_t *d, int w, int h);

extern dialogue_t *viewer_scaledlg;

result_t viewer_scaledlg_init(void);
void viewer_scaledlg_fin(void);

void viewer_scaledlg_set(viewer_t *viewer, int scale, int redraw);

int viewer_scaledlg_fit_to_screen(viewer_t *viewer);

#endif /* VIEWER_SCALEDLG_H */
