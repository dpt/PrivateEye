/* --------------------------------------------------------------------------
 *    Name: vector.h
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

/**
 * \file Vector (interface).
 *
 * Vector is an abstracted array which can be resized by both length and
 * element width.
 *
 * Elements are of a fixed size, stored contiguously and are addressed by
 * index.
 *
 * If the vector is altered then pointers into the vector may be invalidated
 * (should the block move when reallocated).
 */

#ifndef APPENGINE_VECTOR_H
#define APPENGINE_VECTOR_H

#include <stddef.h>

#include "appengine/base/errors.h"

/**
 * A vector.
 */
typedef struct vector_t vector_t;

/* ----------------------------------------------------------------------- */

/**
 * Create a new vector.
 *
 * \param width Byte width of each element.
 *
 * \return New vector, or NULL if out of memory.
 */
vector_t *vector_create(size_t width);

/**
 * Destroy an existing vector.
 *
 * \param doomed Vector to destroy.
 */
void vector_destroy(vector_t *doomed);

/* ----------------------------------------------------------------------- */

/**
 * Clear the specified vector.
 *
 * \param v Vector.
 */
void vector_clear(vector_t *vector);

/* ----------------------------------------------------------------------- */

/**
 * Return the number of elements stored in the specified vector.
 *
 * \param v Vector.
 *
 * \return Number of elements stored.
 */
int vector_length(const vector_t *vector);

/**
 * Change the number of elements stored in the specified vector.
 *
 * \param v      Vector.
 * \param nelems New length.
 *
 * \return Error indication.
 */
error vector_set_length(vector_t *vector, size_t length);

/* ----------------------------------------------------------------------- */

/**
 * Return the byte width of element stored in the specified vector.
 *
 * \param v Vector.
 *
 * \return Byte width of stored elements.
 */
int vector_width(const vector_t *vector);

/**
 * Change the byte width of element stored in the specified vector.
 *
 * If the element width is reduced then any extra bytes are lost. If
 * increased, then zeroes are inserted.
 *
 * \param v     Vector.
 * \param width New element width.
 *
 * \return Error indication.
 */
error vector_set_width(vector_t *vector, size_t width);

/* ----------------------------------------------------------------------- */

/**
 * Retrieve an element of the vector by index.
 *
 * \param v     Vector.
 * \param index Index of element wanted.
 *
 * \return Pointer to element.
 */
void *vector_get(vector_t *vector, int index);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_VECTOR_H */