
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

void window_open_as_menu_here(wimp_w w, int x, int y)
{
  wimp_create_menu((wimp_menu *) w, x, y);
}
