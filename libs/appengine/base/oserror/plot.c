/* --------------------------------------------------------------------------
 *    Name: plot-warn.c
 * Purpose:
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"
#include "oslib/colourtrans.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/base/oserror.h"

void oserror_plot(os_error *e, int x, int y)
{
  os_colour_number c;

  c = colourtrans_return_colour_number(os_COLOUR_WHITE);
  os_set_colour(os_COLOUR_SET_BG, c);
  os_writec(os_VDU_CLG);
  xwimptextop_set_colour(os_COLOUR_BLACK, os_COLOUR_WHITE);
  xwimptextop_paint(0, e->errmess, x + 8, y + 16);
}
