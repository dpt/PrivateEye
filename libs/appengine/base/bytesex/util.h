/* --------------------------------------------------------------------------
 *    Name: util.h
 * Purpose: Utils for reversing bytesex
 * Version: $Id: util.h,v 1.1 2010-01-06 00:36:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef BITWISE_UTIL_H
#define BITWISE_UTIL_H

/* Rotate 'x' right by 'n' bits. */
#define ROR(x, n) ( ((x) >> (n)) | ((x) << (32 - (n))) )

#endif /* BITWISE_UTIL_H */

