/* --------------------------------------------------------------------------
 *    Name: degs-to-rads.c
 * Purpose: Degrees to radians
 * ----------------------------------------------------------------------- */

#include "appengine/base/utils.h"

#define TWICEPI 6.2831853071795864769252867665590

double degs_to_rads(double degs)
{
  return degs * TWICEPI / 360.0;
}
