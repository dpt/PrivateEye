/* --------------------------------------------------------------------------
 *    Name: pixel-size.c
 * Purpose: read_current_pixel_size
 * Version: $Id: pixel-size.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
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
