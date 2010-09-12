/* --------------------------------------------------------------------------
 *    Name: rev-s-m.c
 * Purpose: Reversing bytesex
 * Version: $Id: rev-s-m.c,v 1.2 2010-01-12 23:39:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

unsigned short int rev_s_m(const unsigned char *p)
{
  unsigned short a, b;

  a = p[0];
  b = p[1];

  return (a << 8) | (b << 0);
}
