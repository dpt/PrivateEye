/* --------------------------------------------------------------------------
 *    Name: box.h
 * Purpose: Box operations
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_BOX_H
#define APPENGINE_BOX_H

#include "oslib/os.h"

#define T os_box

/* box handling primitives */
int box_contains_box(const T *inside, const T *outside);
int box_contains_point(const T *b, int x, int y);
int box_is_empty(const T *a);
int box_intersects(const T *a, const T *b);
void box_grow(T *b, int change);
void box_intersection(const T *a, const T *b, T *c);
void box_union(const T *a, const T *b, T *c);

/* Rounds the box's coordinates so that they're a multiple of 'amount'.
 * x0+y0 are rounded down. x1+y1 are rounded up. */
void box_round(T *a, int amount);
void box_round4(T *a);

int box_could_hold(const T *b, int w, int h);
void box_set_origin(T *b, int x0, int y0);

#undef T

#endif /* APPENGINE_BOX_H */
