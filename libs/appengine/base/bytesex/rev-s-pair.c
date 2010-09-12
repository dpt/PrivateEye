/* --------------------------------------------------------------------------
 *    Name: rev-s-pair.c
 * Purpose: Reversing bytesex
 * Version: $Id: rev-s-pair.c,v 1.2 2010-01-12 23:39:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

unsigned int rev_s_pair(unsigned int r0)
{
  unsigned int mask;
  unsigned int r1;

  mask = 0xff00ffffU;

  r1  = mask & (r0 << 8);
  r0 &= ~(mask >> 8);
  r0  = r1 | (r0 >> 8);

  return r0;
}
