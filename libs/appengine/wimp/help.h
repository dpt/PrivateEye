/* --------------------------------------------------------------------------
 *    Name: help.h
 * Purpose: Interactive Help
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_HELP_H
#define APPENGINE_HELP_H

#include "appengine/base/errors.h"

error help_init(void);
void help_fin(void);

error help_add_window(wimp_w w, const char *name);
void help_remove_window(wimp_w w);

error help_add_menu(wimp_menu *m, const char *name);
void help_remove_menu(wimp_menu *m);

#endif /* APPENGINE_HELP_H */
