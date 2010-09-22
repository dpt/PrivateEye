/* --------------------------------------------------------------------------
 *    Name: choices-colour.h
 * Purpose: Colour correction choices header
 * Version: $Id: choices-colour.h,v 1.7 2009-06-11 20:54:29 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef CHOICES_COLOUR_H
#define CHOICES_COLOUR_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#include "appengine/app/choices.h"

extern error colour_choicepane_initialise(const choices_pane *);
extern void colour_choicepane_finalise(const choices_pane *);
extern error colour_choicepane_changed(const choices_pane *);
extern error colour_choicepane_redraw(const choices_pane *, wimp_draw *);

#endif /* CHOICES_COLOUR_H */
