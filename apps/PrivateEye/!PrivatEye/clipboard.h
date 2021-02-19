/* --------------------------------------------------------------------------
 *    Name: clipboard.h
 * Purpose: Clipboard
 * ----------------------------------------------------------------------- */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "appengine/base/errors.h"

result_t clipboard_init(void);
void clipboard_fin(void);

void clipboard_claim(wimp_w w);
void clipboard_release(void);
osbool clipboard_own(void);

#endif /* CLIPBOARD_H */
