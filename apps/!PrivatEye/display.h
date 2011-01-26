/* --------------------------------------------------------------------------
 *    Name: display.h
 * Purpose: Displays
 * Version: $Id: display.h,v 1.12 2009-05-20 21:38:18 dpt Exp $
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
