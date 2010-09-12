/* --------------------------------------------------------------------------
 *    Name: slider.h
 * Purpose: Slider gadgets
 * Version: $Id: slider.h,v 1.1 2008-07-27 18:59:04 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SLIDER_H
#define APPENGINE_SLIDER_H

#include "oslib/wimp.h"

typedef void (slider_update)(wimp_i i, int val);

void slider_start(wimp_pointer *pointer,
                  slider_update *update,
                  int min, int max);

void slider_set(wimp_w w, wimp_i i, int val, int min, int max);

#endif /* APPENGINE_SLIDER_H */

