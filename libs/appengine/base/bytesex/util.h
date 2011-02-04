/* --------------------------------------------------------------------------
 *    Name: util.h
 * Purpose: Utils for reversing bytesex
 * ----------------------------------------------------------------------- */

#ifndef BITWISE_UTIL_H
#define BITWISE_UTIL_H

/* Rotate 'x' right by 'n' bits. */
#define ROR(x, n) ( ((x) >> (n)) | ((x) << (32 - (n))) )

#endif /* BITWISE_UTIL_H */

