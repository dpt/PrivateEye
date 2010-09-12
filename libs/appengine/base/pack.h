/* --------------------------------------------------------------------------
 *    Name: pack.h
 * Purpose: Structure packing and unpacking
 * Version: $Id: pack.h,v 1.3 2010-01-09 21:51:16 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_PACK_H
#define APPENGINE_PACK_H

/**
 * Structure packing.
 *
 * The arguments are packed into 'buf' according to the format string 'fmt'
 * using little-endian byte order.
 *
 * The format string argument accepts the following specifiers:
 *
 *  - 'c' to pack into 8 bits  (notional char)
 *  - 's' to pack into 16 bits (notional short)
 *  - 'i' to pack into 32 bits (notional int)
 *
 * Each specifer may be preceded by a count.
 *
 * Examples:
 *
 *     n = pack(outbuf, "ccc", 1, 2, 3); ("ccc" can also be written "3c")
 *     n = pack(outbuf, "2si", 0x2000, 12345, 1 << 31);
 *
 * Using '*' instead of a count invokes array mode: the next argument is
 * used as an array length and the next after that as an array base pointer.
 *
 * Example:
 *
 *     n = pack(outbuf, "*s", 5, shortarray);
 *
 * Writes out five shorts from shortarray to outbuf.
 *
 * @param[in] outbuf Output buffer to receive packed values.
 * @param[in] fmt    Format string specifiying what to pack.
 *
 * @return Number of bytes used in output buffer.
 */
int pack(unsigned char *outbuf, const char *fmt, ...);

/**
 * Structure unpacking.
 *
 * The arguments are unpacked from 'buf' according to the format string
 * 'fmt' using little-endian byte order.
 *
 * @see pack for a description of the format string.
 *
 * Example:
 *
 *     n = unpack(inbuf, "c3i", &byte1, &byte2, &byte3, &flags);
 *
 * Using '*' instead of a count invokes array mode: the next argument is
 * used as an array length and the next after that as an array base pointer.
 *
 * Example:
 *
 *     n = unpack(inbuf, "*s", 5, shortarray);
 *
 * Writes out five shorts from inbuf to shortarray.
 *
 * @param[in] inbuf Input buffer of packed values.
 * @param[in] fmt   Format string specifiying what to unpack.
 *
 * @return Number of bytes used from input buffer.
 */
int unpack(const unsigned char *inbuf, const char *fmt, ...);

#endif /* APPENGINE_PACK_H */
