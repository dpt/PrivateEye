/* --------------------------------------------------------------------------
 *    Name: rev-s.c
 * Purpose: Reversing bytesex
 * Version: $Id: rev-s.c,v 1.2 2010-01-12 23:39:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

unsigned short int rev_s(unsigned short int r0)
{
  return (r0 >> 8) | (r0 << 8);
}
