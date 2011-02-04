/* --------------------------------------------------------------------------
 *    Name: scale.h
 * Purpose: Scale dialogue
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_SCALE_H
#define APPENGINE_DIALOGUE_SCALE_H

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

dialogue_t *scale__create(void);
void scale__destroy(dialogue_t *d);

void scale__set_bounds(dialogue_t *d, int min, int max);
void scale__set_steppings(dialogue_t *d, int step, int mult);

void scale__set(dialogue_t *d, int scale);
int scale__get(dialogue_t *d);

#define scale__TYPE_VALUE         (0)
#define scale__TYPE_FIT_TO_SCREEN (1)
#define scale__TYPE_FIT_TO_WINDOW (2)

typedef unsigned int scale__type;

typedef void (scale__scale_handler)(dialogue_t *d, scale__type type, int scale);
void scale__set_scale_handler(dialogue_t *d, scale__scale_handler *handler);

#endif /* APPENGINE_DIALOGUE_SCALE_H */
