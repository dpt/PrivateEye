/* --------------------------------------------------------------------------
 *    Name: info.h
 * Purpose: Viewer info dialogue handler
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_INFODLG_H
#define VIEWER_INFODLG_H

#include "appengine/base/errors.h"

extern dialogue_t *viewer_infodlg;
extern dialogue_t *viewer_srcinfodlg;

result_t viewer_infodlg_init(void);
void viewer_infodlg_fin(void);

#endif /* VIEWER_INFODLG_H */
