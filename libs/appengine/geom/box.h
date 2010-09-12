/* --------------------------------------------------------------------------
 *    Name: box.h
 * Purpose: Box operations
 *  Author: David Thomas
 * Version: $Id: box.h,v 1.1 2009-05-18 22:25:31 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_BOX_H
#define APPENGINE_BOX_H

#include "oslib/os.h"

#define T os_box

/* box handling primitives */
int box__contains_box(const T *inside, const T *outside);
int box__contains_point(const T *b, int x, int y);
int box__is_empty(const T *a);
int box__intersects(const T *a, const T *b);
void box__grow(T *b, int change);
void box__intersection(const T *a, const T *b, T *c);
void box__union(const T *a, const T *b, T *c);

/* Rounds the box's coordinates so that they're a multiple of 'amount'.
 * x0+y0 are rounded down. x1+y1 are rounded up. */
void box__round(T *a, int amount);
void box__round4(T *a);

int box__could_hold(const T *b, int w, int h);

#undef T

#endif /* APPENGINE_BOX_H */
