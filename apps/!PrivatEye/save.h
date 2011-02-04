/* --------------------------------------------------------------------------
 *    Name: save.h
 * Purpose: Viewer save dialogue handler
 * Version: $Id: save.h,v 1.10 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_SAVEDLG_H
#define VIEWER_SAVEDLG_H

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

extern dialogue_t *viewer_savedlg;

error viewer_savedlg_init(void);
void viewer_savedlg_fin(void);

#endif /* VIEWER_SAVEDLG_H */
