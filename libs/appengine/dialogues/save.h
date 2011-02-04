/* --------------------------------------------------------------------------
 *    Name: save.h
 * Purpose: Save dialogue
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_SAVE_H
#define APPENGINE_DIALOGUE_SAVE_H

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

dialogue_t *save__create(void);
void save__destroy(dialogue_t *d);

error save__set_file_name(dialogue_t *d, const char *file_name);
void save__set_file_type(dialogue_t *d, bits file_type);

typedef void (save__save_handler)(dialogue_t *d, const char *file_name);
void save__set_save_handler(dialogue_t *d, save__save_handler *handler);

int save__should_close_menu(void);

#endif /* APPENGINE_DIALOGUE_SAVE_H */
