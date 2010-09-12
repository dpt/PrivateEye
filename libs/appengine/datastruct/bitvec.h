/* --------------------------------------------------------------------------
 *    Name: bitvec.h
 * Purpose: Bit vectors
 * Version: $Id: bitvec.h,v 1.3 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

/* Bit vectors are conceptually any length.
 * The unallocated bits are treated as zero. */

#ifndef APPENGINE_BITVEC_H
#define APPENGINE_BITVEC_H

#include "appengine/base/errors.h"

#define T bitvec_t

typedef struct T T;

/* Creates a bit vector big enough to hold 'length' bits.
 * All bits are zero after creation. */
T *bitvec__create(int length);
void bitvec__destroy(T *v);

error bitvec__set(T *v, int bit);
void bitvec__clear(T *v, int bit);
int bitvec__get(const T *v, int bit);
error bitvec__toggle(T *v, int bit);

/* Returns the length of the vector in bits. */
int bitvec__length(const T *v);

/* Returns the number of set bits in the vector. */
int bitvec__count(const T *v);

/* Returns the number of the next set bit after 'n'. */
/* -1 should be the initial value (bits are numbered 0..) */
int bitvec__next(const T *v, int n);

int bitvec__eq(const T *a, const T *b);

error bitvec__and(const T *a, const T *b, T **c);
error bitvec__or(const T *a, const T *b, T **c);

void bitvec__set_all(T *v);
void bitvec__clear_all(T *v);

#undef T

#endif /* APPENGINE_BITVEC_H */
