/* --------------------------------------------------------------------------
 *    Name: txtscr.h
 * Purpose: Text format 'screen'
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TXTSCR_H
#define APPENGINE_TXTSCR_H

#include "oslib/os.h"

#define T txtscr_t

typedef struct T T;

T *txtscr_create(int width, int height);
void txtscr_destroy(T *doomed);

void txtscr_clear(T *scr);

/* adds to the pixels, rather than overwriting */
void txtscr_addbox(T *scr, const os_box *box);

void txtscr_print(T *scr);

#undef T

#endif /* APPENGINE_TXTSCR_H */
