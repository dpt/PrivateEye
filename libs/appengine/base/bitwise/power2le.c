/* --------------------------------------------------------------------------
 *    Name: power2le.c
 * Purpose: Returns the power of two less than or equal to the argument
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int power2le(unsigned int x)
{
    SPREADMSB(x);
    return (x >> 1) + 1;
}
