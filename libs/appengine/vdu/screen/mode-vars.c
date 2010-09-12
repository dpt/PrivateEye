/* --------------------------------------------------------------------------
 *    Name: mode-vars.c
 * Purpose: cache_mode_vars, read_current_mode_vars,
 *          read_screen_dimensions, read_drag_box_for_screen
 * Version: $Id: mode-vars.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"

#include "appengine/vdu/screen.h"

static int mode_vars[5];

void cache_mode_vars(void)
{
  static const os_VDU_VAR_LIST(6) screen_vars =
  {{
    os_MODEVAR_XEIG_FACTOR,
    os_MODEVAR_YEIG_FACTOR,
    os_MODEVAR_LOG2_BPP,
    os_MODEVAR_XWIND_LIMIT,
    os_MODEVAR_YWIND_LIMIT,
    os_VDUVAR_END_LIST
  }};

  os_read_vdu_variables((const os_vdu_var_list *) &screen_vars, mode_vars);
}

void read_current_mode_vars(int *xeig, int *yeig, int *log2bpp)
{
  if (xeig)
    *xeig = mode_vars[0];
  if (yeig)
    *yeig = mode_vars[1];
  if (log2bpp)
    *log2bpp = mode_vars[2];
}

void read_screen_dimensions(int *w, int *h)
{
  *w = (mode_vars[3] + 1) << mode_vars[0];
  *h = (mode_vars[4] + 1) << mode_vars[1];
}

/* Note that these coordinates are inclusive */
void read_drag_box_for_screen(os_box *box)
{
  os_box b;

  b.x0 = 0;
  b.y0 = 0;
  b.x1 = mode_vars[3] << mode_vars[0];
  b.y1 = mode_vars[4] << mode_vars[1];

  *box = b;
}
