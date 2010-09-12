/* --------------------------------------------------------------------------
 *    Name: msb.c
 * Purpose: Returns the most significant set bit
 * Version: $Id: msb.c,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int msb(unsigned int x)
{
    SPREADMSB(x);
    return x & ~(x >> 1);
}
