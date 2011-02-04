/* --------------------------------------------------------------------------
 *    Name: prog-info.h
 * Purpose: ProgInfo window
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_PROGINFO_H
#define APPENGINE_DIALOGUE_PROGINFO_H

#include "appengine/wimp/dialogue.h"

#define proginfo__VERSION_ICON 7

dialogue_t *proginfo__create(void);
void proginfo__destroy(dialogue_t *d);

#endif /* APPENGINE_DIALOGUE_PROGINFO_H */
