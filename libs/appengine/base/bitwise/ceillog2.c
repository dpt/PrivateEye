/* --------------------------------------------------------------------------
 *    Name: ceillog2.c
 * Purpose: Returns the ceiling log2 of the argument
 * Version: $Id: ceillog2.c,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"

#include "util.h"

unsigned int ceillog2(unsigned int x)
{
    unsigned int y;

    y = !ispower2(x); /* 1 if x is not a power of two, 0 otherwise */
    SPREADMSB(x);

    return countbits(x >> 1) + y;
}

