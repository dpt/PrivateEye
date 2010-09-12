/* --------------------------------------------------------------------------
 *    Name: rev-l.c
 * Purpose: Reversing bytesex
 * Version: $Id: rev-l.c,v 1.2 2010-01-12 23:39:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

#include "util.h"

unsigned int rev_l(unsigned int r0)
{
  unsigned int mask;
  unsigned int r1;

  mask = 0xffff00ffU;

  r1 = r0 ^ ROR(r0, 16);
  r1 = mask & (r1 >> 8);
  r0 = r1 ^ ROR(r0, 8);

  return r0;
}
