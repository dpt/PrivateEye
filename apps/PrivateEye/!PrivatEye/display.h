/* --------------------------------------------------------------------------
 *    Name: display.h
 * Purpose: Displays
 * ----------------------------------------------------------------------- */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "appengine/base/errors.h"

#include "viewer.h"

error display_substrate_init(void);

error display_init(void);
void display_fin(void);

error display_set_handlers(viewer_t *viewer);
void display_release_handlers(viewer_t *viewer);

#endif /* DISPLAY_H */
