/* --------------------------------------------------------------------------
 *    Name: save.h
 * Purpose: Save
 * Version: $Id: save.h,v 1.10 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_SAVE_H
#define VIEWER_SAVE_H

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

extern dialogue_t *save;

error viewer_save_init(void);
void viewer_save_fin(void);

#endif /* VIEWER_SAVE_H */
