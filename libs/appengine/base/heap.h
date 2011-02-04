/* --------------------------------------------------------------------------
 *    Name: Heap.h
 * Purpose: Declarations for the heap manager library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_HEAP_H
#define APPENGINE_HEAP_H

#include "kernel.h"

#include <stddef.h> /* for size_t */

/* Heap.c ---------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern char *heap_create(const char *description,
                         size_t size_limit);

/*
 * DocumentMe
 */
extern void heap_delete(char *base);

/*
 * DocumentMe
 */
extern char *heap_claim(char *base,
                        size_t required_size);

/*
 * DocumentMe
 */
extern void heap_release(char *base,
                         char *block);

/* xosheap_resize_r3.s --------------------------------------------------- */

/*
 * DocumentMe
 */
extern _kernel_oserror *xosheap_resize_r3(char *heap,
                                          int required_change,
                                          int *actual_change);

#endif /* APPENGINE_HEAP_H */
