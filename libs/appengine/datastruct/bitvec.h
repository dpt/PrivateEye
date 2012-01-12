/* --------------------------------------------------------------------------
 *    Name: bitvec.h
 * Purpose: Vector of bits
 * ----------------------------------------------------------------------- */

/**
 * \file Bit Vector (interface).
 *
 * Vectors of bits.
 *
 * Bit vectors are an array of bits. They are of variable length and are
 * allocated dynamically.
 *
 * Unallocated bits are notionally always present and read as zero.
 *
 * \see Bit Array for manipulating a pre-allocated bit array.
 */

#ifndef APPENGINE_BITVEC_H
#define APPENGINE_BITVEC_H

#include "appengine/base/errors.h"

#define T bitvec_t

typedef struct T T;

/* Creates a bit vector big enough to hold 'length' bits.
 * All bits are zero after creation. */
T *bitvec_create(int length);
void bitvec_destroy(T *v);

error bitvec_set(T *v, int bit);
void bitvec_clear(T *v, int bit);
error bitvec_toggle(T *v, int bit);

int bitvec_get(const T *v, int bit);

/* Returns the length of the vector in bits. */
int bitvec_length(const T *v);

/* Returns the number of set bits in the vector. */
int bitvec_count(const T *v);

/* Returns the number of the next set bit after 'n'. */
/* -1 should be the initial value (bits are numbered 0..) */
int bitvec_next(const T *v, int n);

int bitvec_eq(const T *a, const T *b);

error bitvec_and(const T *a, const T *b, T **c);
error bitvec_or(const T *a, const T *b, T **c);

void bitvec_set_all(T *v);
void bitvec_clear_all(T *v);

#undef T

#endif /* APPENGINE_BITVEC_H */
