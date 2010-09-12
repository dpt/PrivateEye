/* --------------------------------------------------------------------------
 *    Name: beep.c
 * Purpose: Sounds the system beep
 * Version: $Id: beep.c,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"

#include "appengine/base/os.h"

void beep(void)
{
  os_bell();
}
