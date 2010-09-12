/* --------------------------------------------------------------------------
 *    Name: degs-to-rads.c
 * Purpose: Degrees to radians
 * Version: $Id: degs-to-rads.c,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/utils.h"

#define TWICEPI 6.2831853071795864769252867665590

double degs_to_rads(double degs)
{
  return degs * TWICEPI / 360.0;
}
