/* --------------------------------------------------------------------------
 *    Name: colour.h
 * Purpose: Turning colours to names
 * Version: $Id: colour.h,v 1.1 2009-04-28 23:32:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_COLOUR_H
#define APPENGINE_COLOUR_H

#include "oslib/os.h"

const char *colour_to_name(os_colour c);
char *colour_to_approx(os_colour c);

#endif /* APPENGINE_COLOUR_H */
