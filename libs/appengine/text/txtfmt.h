/* --------------------------------------------------------------------------
 *    Name: txtfmt.h
 * Purpose: Text formatting
 * ----------------------------------------------------------------------- */

/* Word-wrapping is provided by DPTLib's text/txtfmt. This module adds
 * the RISC OS desktop-specific ability to paint the wrapped text. */

#ifndef APPENGINE_TXTFMT_H
#define APPENGINE_TXTFMT_H

#include "oslib/wimp.h"

#include "text/txtfmt.h"

#include "appengine/base/errors.h"

/* Paints the text using the desktop font atop the specified background
 * colour. line_height is the OS unit height of a line of text. */
result_t txtfmt_paint(const txtfmt_t *tx,
                      int             x,
                      int             y,
                      int             line_height,
                      wimp_colour     bgcolour);

#endif /* APPENGINE_TXTFMT_H */
