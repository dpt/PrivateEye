/* --------------------------------------------------------------------------
 *    Name: name.h
 * Purpose: Name dialogue (inputting a single line of text)
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_NAME_H
#define APPENGINE_DIALOGUE_NAME_H

#include "appengine/wimp/dialogue.h"

dialogue_t *name_create(const char *template);

void name_destroy(dialogue_t *d);

void name_set(dialogue_t *d, const char *name);

typedef void (name_ok_handler)(dialogue_t *d,
                                const char *name,
                                void       *arg);

/* Set the handler called when the 'OK' button is clicked. */
void name_set_ok_handler(dialogue_t       *d,
                          name_ok_handler *handler,
                          void             *arg);

#endif /* APPENGINE_DIALOGUE_NAME_H */
