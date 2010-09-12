/* --------------------------------------------------------------------------
 *    Name: power2gt.c
 * Purpose: Returns the power of two greater than the argument
 * Version: $Id: power2gt.c,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int power2gt(unsigned int x)
{
    SPREADMSB(x);
    return x + 1;
}
