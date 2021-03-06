/* --------------------------------------------------------------------------
 *    Name: rev-s-m.c
 * Purpose: Reversing bytesex
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

unsigned short int rev_s_m(const unsigned char *p)
{
  unsigned short a, b;

  a = p[0];
  b = p[1];

  return (a << 8) | (b << 0);
}
