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

result_t save_set_info(dialogue_t *d,
                 const char       *file_name,
                       bits        file_type,
                       size_t      bytes);

/* Called for data transfer saves, i.e. icon dragged to directory display */
typedef void (save_dataxfer_handler)(dialogue_t *d, int my_ref);
void save_set_dataxfer_handler(dialogue_t *d, save_dataxfer_handler *handler);

/* Called for direct saves, i.e. dialogue 'Save' button is clicked */
typedef void (save_save_handler)(dialogue_t *d, const char *file_name);
void save_set_save_handler(dialogue_t *d, save_save_handler *handler);

void save_done(void);

#endif /* APPENGINE_DIALOGUE_SAVE_H */
