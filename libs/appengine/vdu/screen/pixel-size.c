/* --------------------------------------------------------------------------
 *    Name: pixel-size.c
 * Purpose: read_current_pixel_size
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/os.h"

#include "appengine/vdu/screen.h"

void read_current_pixel_size(int *w, int *h)
{
  int xeig,yeig;

  read_current_mode_vars(&xeig, &yeig, NULL);

  *w = 1 << xeig;
  *h = 1 << yeig;
}
