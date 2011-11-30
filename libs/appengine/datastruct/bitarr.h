/* --------------------------------------------------------------------------
 *    Name: bitarr.h
 * Purpose: Bit array
 * ----------------------------------------------------------------------- */

/* Bit arrays are of a fixed length and allocated by the client.
 * This interface performs no bounds checking. */

#ifndef APPENGINE_BITARR_H
#define APPENGINE_BITARR_H

#include <stdlib.h>

#include "oslib/types.h"

/* A bit array's type. */
typedef struct bitarr_t bitarr_t;

/* A bit array's element type. */
typedef unsigned int bitarr_elem_t;

#define BITARR_SHIFT 5 /* log2 (sizeof(bitarr_elem_t) * CHAR_BIT) */
#define BITARR_BITS (1u << BITARR_SHIFT)
#define BITARR_MASK (BITARR_BITS - 1)

/* Return the number of elements required to store the given number of bits. */
#define BITARR_ELEMS(nbits) ((nbits + BITARR_BITS - 1) >> BITARR_SHIFT)

/* Declare a bit array with the specified number of bits. */
#define bitarr_ARRAY(nbits)                     \
  struct {                                      \
    bitarr_elem_t entries[BITARR_ELEMS(nbits)]; \
  }

struct bitarr_t {
  bitarr_elem_t entries[UNKNOWN];
};

/* Clear all bits in the specified bit array. */
#define bitarr__wipe(arr, bytelen)    \
  do {                                \
    memset(&arr.entries, 0, bytelen); \
  } while (0)

/* Set a single bit. */
#define bitarr__set(arr, bit)                                             \
  do {                                                                    \
    (arr)->entries[(bit) >> BITARR_SHIFT] |= 1u << ((bit) & BITARR_MASK); \
  } while (0)

/* Clear a single bit. */
#define bitarr__clear(arr, bit)                                              \
  do {                                                                       \
    (arr)->entries[(bit) >> BITARR_SHIFT] &= ~(1u << ((bit) & BITARR_MASK)); \
  } while (0)

/* Toggle a single bit. */
#define bitarr__toggle(arr, bit)                                          \
  do {                                                                    \
    (arr)->entries[(bit) >> BITARR_SHIFT] ^= 1u << ((bit) & BITARR_MASK); \
  } while (0)

/* Retrieve the value of the specified bit. */
#define bitarr__get(arr, bit) \
  (!!((arr)->entries[(bit) >> BITARR_SHIFT] & (1u << ((bit) & BITARR_MASK))))

/* Returns the number of set bits in the array. */
int bitarr__count(const struct bitarr_t *arr, size_t bytelen);

#endif /* APPENGINE_BITARR_H */
