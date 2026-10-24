/* --------------------------------------------------------------------------
 *    Name: tonemap-gadget.h
 * Purpose: ToneMap gadget
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TONEMAP_GADGET_H
#define APPENGINE_TONEMAP_GADGET_H

#include "oslib/wimp.h"

#include "framebuf/tonemap.h"

/* Draws one combined curve when R, G and B are in sync, otherwise separate
 * curves, plus an alpha curve when the map has alpha. stroke_width is the
 * width of the curves in OS units. */
int tonemapgadget_update(tonemap_t *map,
                         int        stroke_width,
                         wimp_w     w,
                         wimp_i     i);
int tonemapgadget_redraw(tonemap_t *map,
                         int        stroke_width,
                         wimp_w     w,
                         wimp_i     i,
                         wimp_draw *redraw);

#endif /* APPENGINE_TONEMAP_GADGET_H */
