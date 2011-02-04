/* --------------------------------------------------------------------------
 *    Name: read-max-vis-area.c
 * Purpose: read_max_visible_area
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/vdu/screen.h"

#include "appengine/wimp/window.h"

void read_max_visible_area(wimp_w win, int *w, int *h)
{
  int    sw,sh;
  os_box furn;

  read_screen_dimensions(&sw, &sh);

  read_furniture_dimensions(win, &furn);

  /* compensate screen dimensions for scroll bars (so the window once fit
   * to screen will not have any invisible work area) */
  *w = sw - (furn.x0 + furn.x1);
  *h = sh - (furn.y0 + furn.y1);
}
