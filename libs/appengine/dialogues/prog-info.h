/* --------------------------------------------------------------------------
 *    Name: prog-info.h
 * Purpose: ProgInfo window
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_PROGINFO_H
#define APPENGINE_DIALOGUE_PROGINFO_H

#include "appengine/wimp/dialogue.h"

#define proginfo_VERSION_ICON 7

dialogue_t *proginfo_create(void);
void proginfo_destroy(dialogue_t *d);

#endif /* APPENGINE_DIALOGUE_PROGINFO_H */
