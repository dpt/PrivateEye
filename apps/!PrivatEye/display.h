/* --------------------------------------------------------------------------
 *    Name: display.h
 * Purpose: Displays
 * ----------------------------------------------------------------------- */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "appengine/base/errors.h"

#include "viewer.h"

error display__init(void);
void display__fin(void);
error display__set_handlers(viewer_t *viewer);
void display__release_handlers(viewer_t *viewer);

#endif /* DISPLAY_H */
