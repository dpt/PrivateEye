
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/base/oserror.h"

#include "appengine/wimp/window.h"

wimp_w window_create_from_memory(wimp_window *window)
{
  osspriteop_area *sprite_area;
  wimp_w           w;

  sprite_area = window_get_sprite_area();

  window->sprite_area = (sprite_area) ? sprite_area : wimpspriteop_AREA;

  EC(xwimp_create_window(window, &w));

  return w;
}
