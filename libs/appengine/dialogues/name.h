/* --------------------------------------------------------------------------
 *    Name: name.h
 * Purpose: Name dialogue (inputting a single line of text)
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_NAME_H
#define APPENGINE_DIALOGUE_NAME_H

#include "appengine/wimp/dialogue.h"

dialogue_t *name__create(const char *template);

void name__destroy(dialogue_t *d);

void name__set(dialogue_t *d, const char *name);

typedef void (name__ok_handler)(dialogue_t *d,
                                const char *name,
                                void       *arg);

/* Set the handler called when the 'OK' button is clicked. */
void name__set_ok_handler(dialogue_t       *d,
                          name__ok_handler *handler,
                          void             *arg);

#endif /* APPENGINE_DIALOGUE_NAME_H */
