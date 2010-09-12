/* --------------------------------------------------------------------------
 *    Name: rev-s-pair-m.c
 * Purpose: Reversing bytesex
 * Version: $Id: rev-s-pair-m.c,v 1.2 2010-01-12 23:39:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bytesex.h"

unsigned int rev_s_pair_m(const unsigned char *p)
{
  unsigned int a, b, c, d;

  a = p[0];
  b = p[1];
  c = p[2];
  d = p[3];

  return (a << 8) | (b << 0) | (c << 24) | (d << 16);
}
