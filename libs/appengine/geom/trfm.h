/* --------------------------------------------------------------------------
 *    Name: trfm.h
 * Purpose: Routines for dealing with transforms
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TRFM_H
#define APPENGINE_TRFM_H

#include "oslib/os.h"

#define T os_trfm

void trfm_set_identity(T *t);
void trfm_update(T *a, const T *b);
/* angle is in 16.16 degrees */
void trfm_rotate_degs(T *t, int angle);
/* angle is in 16.16 radians */
void trfm_rotate_rads(T *t, int angle);
void trfm_translate(T *transform, int x, int y);

#undef T

#endif /* APPENGINE_TRFM_H */
