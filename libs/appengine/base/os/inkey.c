/* --------------------------------------------------------------------------
 *    Name: inkey.c
 * Purpose: BASIC INKEY equivalent
 * Version: $Id: inkey.c,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/osbyte.h"

#include "appengine/base/os.h"

int inkey(int key)
{
  return osbyte1(osbyte_SCAN_KEYBOARD, key, 0) == 0xff;
}
