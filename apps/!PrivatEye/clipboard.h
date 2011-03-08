/* --------------------------------------------------------------------------
 *    Name: clipboard.h
 * Purpose: Clipboard
 * ----------------------------------------------------------------------- */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "oslib/types.h"
#include "oslib/wimp.h"

error clipboard__init(void);
void clipboard__fin(void);

void clipboard_claim(wimp_w w);
void clipboard_release(void);
osbool clipboard_own(void);

#endif /* CLIPBOARD_H */
