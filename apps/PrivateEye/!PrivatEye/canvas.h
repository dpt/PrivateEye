/* --------------------------------------------------------------------------
 *    Name: canvas.h
 * Purpose: Canvas
 * ----------------------------------------------------------------------- */

#ifndef CANVAS_H
#define CANVAS_H

#include "appengine/base/errors.h"

typedef struct canvas canvas_t;

result_t canvas_substrate_init(void);

result_t canvas_init(void);
void canvas_fin(void);

result_t canvas_create(canvas_t **new_canvas);
void canvas_destroy(canvas_t *doomed);

void canvas_open(canvas_t *canvas);

void canvas_close_all(void);

int canvas_get_count(void);

#endif /* CANVAS_H */
