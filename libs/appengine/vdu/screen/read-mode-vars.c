/* --------------------------------------------------------------------------
 *    Name: read-mode-vars.c
 * Purpose: read_mode_vars
 * Version: $Id: read-mode-vars.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
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
