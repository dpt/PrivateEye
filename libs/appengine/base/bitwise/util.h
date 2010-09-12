/* --------------------------------------------------------------------------
 *    Name: util.h
 * Purpose: Utils for twiddle ops
 * Version: $Id: util.h,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef BITWISE_UTIL_H
#define BITWISE_UTIL_H

/* Spread the most significant set bit downwards so it fills all lower bits.
 */
#define SPREADMSB(x) x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16

#endif /* BITWISE_UTIL_H */

