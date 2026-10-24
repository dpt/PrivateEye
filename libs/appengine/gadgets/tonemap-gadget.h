/* --------------------------------------------------------------------------
 *    Name: tonemap-gadget.h
 * Purpose: ToneMap gadget
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TONEMAP_GADGET_H
#define APPENGINE_TONEMAP_GADGET_H

#include "oslib/wimp.h"

#include "framebuf/tonemap.h"

/* channels is the channel selection being edited: R, G or B alone draws
 * separate curves, all of RGB draws one combined curve, and alpha adds the
 * alpha curve. stroke_width is the width of the curves in OS units. */
int tonemapgadget_update(tonemap_t         *map,
                         tonemap_channels_t channels,
                         int                stroke_width,
                         wimp_w             w,
                         wimp_i             i);
int tonemapgadget_redraw(tonemap_t         *map,
                         tonemap_channels_t channels,
                         int                stroke_width,
                         wimp_w             w,
                         wimp_i             i,
                         wimp_draw         *redraw);

#endif /* APPENGINE_TONEMAP_GADGET_H */
