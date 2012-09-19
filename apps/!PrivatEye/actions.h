/* --------------------------------------------------------------------------
 *    Name: actions.h
 * Purpose: Common actions header
 * ----------------------------------------------------------------------- */

#ifndef ACTIONS_H
#define ACTIONS_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

error action_help(void);
error action_close_window(wimp_w w);

#endif /* ACTIONS_H */
