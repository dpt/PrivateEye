/* --------------------------------------------------------------------------
 *    Name: beep.c
 * Purpose: Sounds the system beep
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"

#include "appengine/base/os.h"

void beep(void)
{
  os_bell();
}
