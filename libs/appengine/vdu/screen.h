/* --------------------------------------------------------------------------
 *    Name: screen.h
 * Purpose: Screen
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_SCREEN_H
#define APPENGINE_SCREEN_H

#include "oslib/os.h"

/* Reduce and set scale factors from the ratio m:n */
void os_factors_from_ratio(os_factors *factors, int m, int n);

/* Reads mode variables in advance */
void cache_mode_vars(void);

/* Reads common mode variables */
void read_current_mode_vars(int *xeig, int *yeig, int *log2bpp);
void read_mode_vars(os_mode m, int *xeig, int *yeig, int *log2bpp);

/* Reads the size of a pixel in OS units */
void read_current_pixel_size(int *w, int *h);

/* Reads the screen width and height, in OS units */
void read_screen_dimensions(int *w, int *h);

/* Reads the screen dimensions suitable for using as Wimp_DragBox's bounds */
void read_drag_box_for_screen(os_box *box);

os_error *screen_clip(const os_box *b);

/**
 * Enumerates all valid numbered graphics modes from 0-127. Each one is
 * scored and the one with the smallest number of wasted pixels, and closest
 * colour depth, is returned.
 */
os_mode closest_mode(int min_width, int min_height, int pref_log2bpp);

/* Sets the screen mode then calls cache_mode_vars. */
void set_mode(os_mode mode);

#endif /* APPENGINE_SCREEN_H */
