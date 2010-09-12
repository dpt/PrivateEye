/* --------------------------------------------------------------------------
 *    Name: prog-info.h
 * Purpose: ProgInfo window
 * Version: $Id: prog-info.h,v 1.2 2009-05-20 21:20:41 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_PROGINFO_H
#define APPENGINE_DIALOGUE_PROGINFO_H

#include "appengine/wimp/dialogue.h"

#define proginfo__VERSION_ICON 7

dialogue_t *proginfo__create(void);
void proginfo__destroy(dialogue_t *d);

#endif /* APPENGINE_DIALOGUE_PROGINFO_H */
