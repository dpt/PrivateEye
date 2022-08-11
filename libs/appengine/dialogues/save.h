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

dialogue_t *save_create(void);
void save_destroy(dialogue_t *d);

result_t save_set_file_name(dialogue_t *d, const char *file_name);
void save_set_file_type(dialogue_t *d, bits file_type);
void save_set_file_size(dialogue_t *d, size_t bytes);

typedef void (save_save_handler)(dialogue_t *d, const char *file_name);
void save_set_save_handler(dialogue_t *d, save_save_handler *handler);

int save_should_close_menu(void);

#endif /* APPENGINE_DIALOGUE_SAVE_H */
