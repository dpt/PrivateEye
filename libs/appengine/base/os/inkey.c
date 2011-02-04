/* --------------------------------------------------------------------------
 *    Name: inkey.c
 * Purpose: BASIC INKEY equivalent
 * ----------------------------------------------------------------------- */

#include "oslib/osbyte.h"

#include "appengine/base/os.h"

int inkey(int key)
{
  return osbyte1(osbyte_SCAN_KEYBOARD, key, 0) == 0xff;
}
