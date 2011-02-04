/* --------------------------------------------------------------------------
 *    Name: read-mode-vars.c
 * Purpose: read_mode_vars
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"

#include "appengine/vdu/screen.h"

void read_mode_vars(os_mode m, int *xeig, int *yeig, int *log2bpp)
{
  if (xeig)
    os_read_mode_variable(m, os_MODEVAR_XEIG_FACTOR, xeig);
  if (yeig)
    os_read_mode_variable(m, os_MODEVAR_YEIG_FACTOR, yeig);
  if (log2bpp)
    os_read_mode_variable(m, os_MODEVAR_LOG2_BPP, log2bpp);
}
