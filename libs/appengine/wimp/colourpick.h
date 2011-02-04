/* --------------------------------------------------------------------------
 *    Name: ColourPick.h
 * Purpose: Declarations for the ColourPicker library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_COLOURPICK_H
#define APPENGINE_COLOURPICK_H

#include "oslib/colourpicker.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

colourpicker_d colourpick_popup(wimp_w w, wimp_i i, os_colour colour);

#endif /* APPENGINE_COLOURPICK_H */
