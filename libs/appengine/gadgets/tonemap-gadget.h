/* --------------------------------------------------------------------------
 *    Name: tonemap-gadget.h
 * Purpose: ToneMap gadget
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TONEMAP_GADGET_H
#define APPENGINE_TONEMAP_GADGET_H

#include "oslib/wimp.h"

#include "appengine/graphics/tonemap.h"

int tonemapgadget_update(tonemap *map, wimp_w w, wimp_i i);
int tonemapgadget_redraw(tonemap *map, wimp_w w, wimp_i i, wimp_draw *redraw);

#endif /* APPENGINE_TONEMAP_GADGET_H */
