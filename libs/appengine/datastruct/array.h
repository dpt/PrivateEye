/* --------------------------------------------------------------------------
 *    Name: array.h
 * Purpose: Array utilities
 * ----------------------------------------------------------------------- */

/**
 * \file Array (interface).
 *
 * Utilities for operating on arrays.
 */

#ifndef APPENGINE_ARRAY_H
#define APPENGINE_ARRAY_H

#include <stdlib.h>

/* Delete the specified element. */
void array_delete_element(void  *array,
                          size_t elemsize,
                          int    nelems,
                          int    doomed);

/* Delete the specified elements. */
/* last_doomed is inclusive. */
void array_delete_elements(void  *array,
                           size_t elemsize,
                           int    nelems,
                           int    first_doomed,
                           int    last_doomed);

/* Take the contents of an array which used to have elements 'oldwidth' bytes
 * wide and adjust them so they are 'newwidth' bytes wide. Set new bytes to
 * 'wipe_value'. */
void array_stretch1(unsigned char *base,
                    int            nelems,
                    size_t         oldwidth,
                    size_t         newwidth,
                    int            wipe_value);

void array_stretch2(unsigned char *base,
                    int            nelems,
                    size_t         oldwidth,
                    size_t         newwidth,
                    int            wipe_value);

/* Take the contents of an array which used to have elements 'oldwidth' bytes
 * wide and adjust them so they are 'newwidth' bytes wide. */
void array_squeeze1(unsigned char *base,
                    int            nelems,
                    size_t         oldwidth,
                    size_t         newwidth);

void array_squeeze2(unsigned char *base,
                    int            nelems,
                    size_t         oldwidth,
                    size_t         newwidth);

/* Temporary defines until the above functions are renamed. */
#define array_stretch array_stretch2
#define array_squeeze array_squeeze2

#endif /* APPENGINE_ARRAY_H */
