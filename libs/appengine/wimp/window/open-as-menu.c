
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_open_as_menu(wimp_w w)
{
  wimp_window_info info;
  int              width, height;
  wimp_pointer     pointer;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  width  = info.extent.x1 - info.extent.x0;
  height = info.extent.y1 - info.extent.y0;

  wimp_get_pointer_info(&pointer);

  wimp_create_menu((wimp_menu *) w,  pointer.pos.x - width / 2,
                                     pointer.pos.y + height / 2);
}
