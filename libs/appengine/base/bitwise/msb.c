/* --------------------------------------------------------------------------
 *    Name: msb.c
 * Purpose: Returns the most significant set bit
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int msb(unsigned int x)
{
    SPREADMSB(x);
    return x & ~(x >> 1);
}
