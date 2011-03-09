/* --------------------------------------------------------------------------
 *    Name: dataxfer.h
 * Purpose: Data transfer
 * ----------------------------------------------------------------------- */

#ifndef DATAXFER_H
#define DATAXFER_H

#include "oslib/types.h"

#include "appengine/base/errors.h"

error dataxfer__init(void);
void dataxfer__fin(void);

#endif /* DATAXFER_H */
