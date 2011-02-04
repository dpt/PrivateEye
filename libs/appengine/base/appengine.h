/* --------------------------------------------------------------------------
 *    Name: AppEngine.h
 * Purpose: Stuff for dealing with the AppEngine Heap and AppEngine Module
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_APPENGINE_H
#define APPENGINE_APPENGINE_H

#include "kernel.h"

#include <stddef.h>

/* Declarations */

/* Resource.c -------------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern const char *resource_locate(const char *);


/* Heap.c ------------------------------------------------------------------------ */

typedef struct ae_heap
{
  int hp_guardword;
  int hp_dynamic;
  int hp_usedanchors;
  int hp_freeanchors;
  int hp_size;
  int hp_free;
  int hp_slotsize;
}
ae_heap;

typedef void **ae_anchor;

/*
 * DocumentMe
 */
extern ae_heap *ae_heap_create_dynamic(const char *);

/*
 * DocumentMe
 */
extern void ae_heap_delete(ae_heap *);

/*
 * DocumentMe
 */
extern ae_anchor ae_heap_claim(ae_heap *,
                               size_t);

/*
 * DocumentMe
 */
extern void ae_heap_release(ae_heap *,
                               ae_anchor *);

/*
 * DocumentMe
 */
extern int ae_heap_resize(ae_heap *,
                          ae_anchor,
                          size_t);

/*
 * DocumentMe
 */
extern size_t ae_heap_size(ae_heap *,
                           ae_anchor);


/* Resource2.s ------------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_resource_op_locate(const char *,
                                                      const char **);

/* Codec2.s ---------------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_codec_op_encode_base64(const char *,
                                                          int,
                                                          const char **);

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_codec_op_decode_base64(const char *,
                                                          int,
                                                          const char **);

/* Heap2.s ----------------------------------------------------------------------- */

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_heap_claim(ae_heap *,
                                              int,
                                              ae_anchor *);

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_heap_create_dynamic(const char *,
                                                       ae_heap **);

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_heap_delete(ae_heap *);

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_heap_release(ae_heap *,
                                                ae_anchor *);

/*
 * DocumentMe
 */
extern _kernel_oserror *xappengine_heap_resize(ae_heap *,
                                               ae_anchor,
                                               int,
                                               int *);

/*
 * Helpful _swi/_swix definitions
 */

/*
#define I0 _IN(0)
#define I1 _IN(1)
#define I2 _IN(2)
#define I3 _IN(3)
#define I4 _IN(4)
#define I5 _IN(5)
#define I6 _IN(6)
#define I7 _IN(7)
#define I8 _IN(8)
#define I9 _IN(9)

#define O0 _OUT(0)
#define O1 _OUT(1)
#define O2 _OUT(2)
#define O3 _OUT(3)
#define O4 _OUT(4)
#define O5 _OUT(5)
#define O6 _OUT(6)
#define O7 _OUT(7)
#define O8 _OUT(8)
#define O9 _OUT(9)

#define R0 _RETURN(0)
#define R1 _RETURN(1)
#define R2 _RETURN(2)
#define R3 _RETURN(3)
#define R4 _RETURN(4)
#define R5 _RETURN(5)
#define R6 _RETURN(6)
#define R7 _RETURN(7)
#define R8 _RETURN(8)
#define R9 _RETURN(9)

#define B0 _BLOCK(0)
#define B1 _BLOCK(1)
#define B2 _BLOCK(2)
#define B3 _BLOCK(3)
#define B4 _BLOCK(4)
#define B5 _BLOCK(5)
#define B6 _BLOCK(6)
#define B7 _BLOCK(7)
#define B8 _BLOCK(8)
#define B9 _BLOCK(9)
*/

#endif /* APPENGINE_APPENGINE_H */
