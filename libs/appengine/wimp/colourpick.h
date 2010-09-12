/* --------------------------------------------------------------------------
 *    Name: ColourPick.h
 * Purpose: Declarations for the ColourPicker library
 *  Author: David Thomas
 * Version: $Id: colourpick.h,v 1.1 2009-06-11 21:20:26 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_COLOURPICK_H
#define APPENGINE_COLOURPICK_H

#include "oslib/colourpicker.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

colourpicker_d colourpick_popup(wimp_w w, wimp_i i, os_colour colour);

#endif /* APPENGINE_COLOURPICK_H */
