/* --------------------------------------------------------------------------
 *    Name: canvas.h
 * Purpose: Canvas
 * ----------------------------------------------------------------------- */

#ifndef CANVAS_H
#define CANVAS_H

#include "appengine/base/errors.h"

typedef struct canvas canvas_t;

error canvas_substrate_init(void);

error canvas_init(void);
void canvas_fin(void);

error canvas_create(canvas_t **new_canvas);
void canvas_destroy(canvas_t *doomed);

void canvas_open(canvas_t *canvas);

void canvas_close_all(void);

int canvas_get_count(void);

#endif /* CANVAS_H */
