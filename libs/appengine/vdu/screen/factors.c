/* --------------------------------------------------------------------------
 *    Name: factors.c
 * Purpose: os_factors_from_ratio
 * Version: $Id: factors.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"

#include "appengine/base/utils.h"

#include "appengine/vdu/screen.h"

void os_factors_from_ratio(os_factors *factors, int mul, int div)
{
  int d;

  /* reduce the scale factors */
  d = gcd(mul, div);
  mul /= d;
  div /= d;

  factors->xmul = factors->ymul = mul;
  factors->xdiv = factors->ydiv = div;
}
