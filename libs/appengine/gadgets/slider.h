/* --------------------------------------------------------------------------
 *    Name: slider.h
 * Purpose: Slider gadgets
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SLIDER_H
#define APPENGINE_SLIDER_H

#include "oslib/wimp.h"

typedef void (slider_update)(wimp_i i, int val, void *opaque);

void slider_start(wimp_pointer  *pointer,
                  int            min,
                  int            max,
                  slider_update *update,
                  void          *opaque);

void slider_set(wimp_w w, wimp_i i, int val, int min, int max);

#endif /* APPENGINE_SLIDER_H */

