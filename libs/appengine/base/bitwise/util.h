/* --------------------------------------------------------------------------
 *    Name: util.h
 * Purpose: Utils for twiddle ops
 * ----------------------------------------------------------------------- */

#ifndef BITWISE_UTIL_H
#define BITWISE_UTIL_H

/* Spread the most significant set bit downwards so it fills all lower bits.
 */
#define SPREADMSB(x) x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16

#endif /* BITWISE_UTIL_H */

