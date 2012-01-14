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

dialogue_t *scale_create(void);
void scale_destroy(dialogue_t *d);

void scale_set_range(dialogue_t *d, int min, int max);
void scale_set_steppings(dialogue_t *d, int step, int mult);

void scale_set(dialogue_t *d, int scale);
int scale_get(dialogue_t *d);

#define scale_TYPE_VALUE         (0)
#define scale_TYPE_FIT_TO_SCREEN (1)
#define scale_TYPE_FIT_TO_WINDOW (2)

typedef unsigned int scale_type;

typedef void (scale_scale_handler)(dialogue_t *d,
                                   scale_type  type,
                                   int         scale);
void scale_set_scale_handler(dialogue_t *d, scale_scale_handler *handler);

#endif /* APPENGINE_DIALOGUE_SCALE_H */
