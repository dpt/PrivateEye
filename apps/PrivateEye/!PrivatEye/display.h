/* --------------------------------------------------------------------------
 *    Name: display.h
 * Purpose: Displays
 * ----------------------------------------------------------------------- */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "appengine/base/errors.h"

#include "viewer.h"

result_t display_substrate_init(void);

result_t display_init(void);
void display_fin(void);

result_t display_set_handlers(viewer_t *viewer);
void display_release_handlers(viewer_t *viewer);

#endif /* DISPLAY_H */
