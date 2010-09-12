/* --------------------------------------------------------------------------
 *    Name: tonemap-gadget.h
 * Purpose: ToneMap gadget
 * Version: $Id: tonemap-gadget.h,v 1.2 2009-04-28 23:32:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TONEMAP_GADGET_H
#define APPENGINE_TONEMAP_GADGET_H

#include "oslib/wimp.h"

#include "appengine/graphics/tonemap.h"

int tonemapgadget_update(tonemap *map, wimp_w w, wimp_i i);
int tonemapgadget_redraw(tonemap *map, wimp_w w, wimp_i i, wimp_draw *redraw);

#endif /* APPENGINE_TONEMAP_GADGET_H */
