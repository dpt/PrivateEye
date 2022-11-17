/* --------------------------------------------------------------------------
 *    Name: open.h
 * Purpose: Internal definitions for the window library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef WINDOW_OPEN
#define WINDOW_OPEN

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

typedef struct window_open_args
{
  int              screen_w, screen_h;
  wimp_window_info info;
  wimp_outline     outline;
  int              left, bottom, right, top; /* should all be +ve */
}
window_open_args_t;

void window_open_with_callback(wimp_w w,
                               void (*cb)(window_open_args_t *, void *),
                               void  *opaque);

void centre_scrollbars(wimp_window_info *info);

#endif /* WINDOW_OPEN */
