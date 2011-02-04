/* --------------------------------------------------------------------------
 *    Name: power2gt.c
 * Purpose: Returns the power of two greater than the argument
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int power2gt(unsigned int x)
{
    SPREADMSB(x);
    return x + 1;
}
