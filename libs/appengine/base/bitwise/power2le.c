/* --------------------------------------------------------------------------
 *    Name: power2le.c
 * Purpose: Returns the power of two less than or equal to the argument
 * Version: $Id: power2le.c,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int power2le(unsigned int x)
{
    SPREADMSB(x);
    return (x >> 1) + 1;
}
