/* --------------------------------------------------------------------------
 *    Name: actions.h
 * Purpose: Common actions header
 * ----------------------------------------------------------------------- */

#ifndef ACTIONS_H
#define ACTIONS_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

result_t action_help(void);
result_t action_close_window(wimp_w w);

#endif /* ACTIONS_H */
