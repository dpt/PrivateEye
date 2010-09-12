/* $Id: redraw.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_redraw(wimp_w w)
{
  wimp_window_info info;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  wimp_force_redraw(w, info.extent.x0,
                       info.extent.y0,
                       info.extent.x1,
                       info.extent.y1);
}
