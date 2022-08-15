/* --------------------------------------------------------------------------
 *    Name: save.h
 * Purpose: Viewer save dialogue handler
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_SAVEDLG_H
#define VIEWER_SAVEDLG_H

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

#include "viewer.h"

extern dialogue_t *viewer_savedlg; // exposed for dialogue_get_window use

viewer_t *viewer_savedlg_get(void);
/* Call when the savedlg is closed. */
void viewer_savedlg_reset(void);

result_t viewer_savedlg_init(void);
void viewer_savedlg_fin(void);

#endif /* VIEWER_SAVEDLG_H */
