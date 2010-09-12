/* --------------------------------------------------------------------------
 *    Name: txtfmt.h
 * Purpose: Text formatting
 * Version: $Id: txtfmt.h,v 1.2 2010-01-13 18:41:09 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TXTFMT_H
#define APPENGINE_TXTFMT_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#define T txtfmt_t

typedef struct T T;

error txtfmt__create(const char *s, T **tx);
void txtfmt__destroy(T *tx);

/* Wrap the string to the specified character width. */
error txtfmt__wrap(T *tx, int width);

/* Paints the text using the desktop font atop the specified background
 * colour. */
error txtfmt__paint(const T *tx, int x, int y, wimp_colour bgcolour);

/* Sets the OS unit height of text use when painting. */
void txtfmt__set_line_height(T *tx, int line_height);

/* Prints the text via printf including line numbers (for testing). */
error txtfmt__print(const T *tx);

int txtfmt__get_height(const T *tx);

/* Returns the length of the string the txtfmt holds. */
int txtfmt__get_length(const T *tx);

/* Returns the wrapped width of a txtfmt.
 * e.g. If you wrap some text containing a word 10 characters long it'll
 *      never get any thinner than 10. */
int txtfmt__get_wrapped_width(const T *tx);

#undef T

#endif /* APPENGINE_TXTFMT_H */
