/* $Id: open.h,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#ifndef WINDOW_OPEN
#define WINDOW_OPEN

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

typedef struct
{
  int              screen_w, screen_h;
  wimp_window_info info;
  wimp_outline     outline;
  int              left, bottom, right, top; /* should all be +ve */
}
CallbackArgs;

void window_open_with_callback(wimp_w  w,
                               void  (*cb)(CallbackArgs *, void *),
                               void   *arg);

void centre_scrollbars(wimp_window_info *info);

#endif /* WINDOW_OPEN */
