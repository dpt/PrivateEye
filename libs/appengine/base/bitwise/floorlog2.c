/* --------------------------------------------------------------------------
 *    Name: floorlog2.c
 * Purpose: Returns the floor log2 of the argument
 * Version: $Id: floorlog2.c,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int floorlog2(unsigned int x)
{
    SPREADMSB(x);
    return countbits(x >> 1);
}
