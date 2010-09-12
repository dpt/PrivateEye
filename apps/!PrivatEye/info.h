/* --------------------------------------------------------------------------
 *    Name: info.h
 * Purpose: Info
 * Version: $Id: info.h,v 1.9 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_INFO_H
#define VIEWER_INFO_H

#include "appengine/base/errors.h"

extern dialogue_t *info;
extern dialogue_t *source_info;

error viewer_info_init(void);
void viewer_info_fin(void);

#endif /* VIEWER_INFO_H */
