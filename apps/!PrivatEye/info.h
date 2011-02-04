/* --------------------------------------------------------------------------
 *    Name: info.h
 * Purpose: Viewer info dialogue handler
 * Version: $Id: info.h,v 1.9 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_INFODLG_H
#define VIEWER_INFODLG_H

#include "appengine/base/errors.h"

extern dialogue_t *viewer_infodlg;
extern dialogue_t *viewer_srcinfodlg;

error viewer_infodlg_init(void);
void viewer_infodlg_fin(void);

#endif /* VIEWER_INFODLG_H */
