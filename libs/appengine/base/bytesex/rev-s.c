/* --------------------------------------------------------------------------
 *    Name: rev-s.c
 * Purpose: Reversing bytesex
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

unsigned short int rev_s(unsigned short int r0)
{
  return (r0 >> 8) | (r0 << 8);
}
