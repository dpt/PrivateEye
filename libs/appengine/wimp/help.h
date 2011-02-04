/* --------------------------------------------------------------------------
 *    Name: help.h
 * Purpose: Interactive Help
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_HELP_H
#define APPENGINE_HELP_H

#include "appengine/base/errors.h"

error help__init(void);
void help__fin(void);

error help__add_window(wimp_w w, const char *name);
void help__remove_window(wimp_w w);

error help__add_menu(wimp_menu *m, const char *name);
void help__remove_menu(wimp_menu *m);

#endif /* APPENGINE_HELP_H */
