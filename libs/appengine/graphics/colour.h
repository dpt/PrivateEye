/* --------------------------------------------------------------------------
 *    Name: colour.h
 * Purpose: Turning colours to names
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_COLOUR_H
#define APPENGINE_COLOUR_H

#include "oslib/os.h"

const char *colour_to_name(os_colour c);
char *colour_to_approx(os_colour c);

#endif /* APPENGINE_COLOUR_H */
