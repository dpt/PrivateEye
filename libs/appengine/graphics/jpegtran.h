/* --------------------------------------------------------------------------
 *    Name: jpegtran.h
 * Purpose: JPEG cleaner header
 * Version: $Id: jpegtran.h,v 1.1 2009-04-28 23:32:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_JPEGTRAN_H
#define APPENGINE_JPEGTRAN_H

#include <stdlib.h>

int jpegtran_clean(const unsigned char *data,
                   size_t               length,
                   unsigned char      **newdata,
                   size_t              *newlength);


/*  THIS                 | OR THIS                      | AKA
 * ----------------------+------------------------------+------------
 *  (none)               | Flip H, Flip V, Rotate 180   |
 *  Rotate 90 ac         | Flip H, Flip V, Rotate 90 cw |
 *  Rotate 180           | Flip H, Flip V               |
 *  Rotate 90 cw         | Flip H, Flip V, Rotate 90 ac |
 *  Flip H               | Flip V, Rotate 180           |
 *  Flip H, Rotate 90 ac | Flip V, Rotate 90 ac         | Transpose
 *  Flip H, Rotate 180   | Flip V                       |
 *  Flip H, Rotate 90 cw | Flip V, Rotate 90 cw         | Transverse
 * ----------------------+------------------------------+------------ */

enum
{
  jpegtran_TRANSFORM_NONE,
  jpegtran_TRANSFORM_ROTATE_90_A,
  jpegtran_TRANSFORM_ROTATE_180,
  jpegtran_TRANSFORM_ROTATE_90_C,
  jpegtran_TRANSFORM_FLIP_H,
  jpegtran_TRANSFORM_TRANSPOSE,
  jpegtran_TRANSFORM_FLIP_V,
  jpegtran_TRANSFORM_TRANSVERSE,

  jpegtran_TRANSFORM_FLAG_TRIM = 1 << 3
};

typedef unsigned int jpegtran_transform_type;

int jpegtran_transform(const unsigned char      *data,
                       size_t                    length,
                       unsigned char           **newdata,
                       size_t                   *newlength,
                       jpegtran_transform_type   args);

/* Retrieve stored error messages. */
const char *jpegtran_get_messages(void);

/* Discard stored error messages once they're used. */
void jpegtran_discard_messages(void);

#endif /* APPENGINE_JPEGTRAN_H */
