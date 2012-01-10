/* --------------------------------------------------------------------------
 *    Name: floorlog2.c
 * Purpose: Returns the floor log2 of the argument
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int floorlog2(unsigned int x)
{
  SPREADMSB(x);
  return countbits(x >> 1);
}
