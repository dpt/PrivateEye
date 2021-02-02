/* --------------------------------------------------------------------------
 *    Name: keymap.h
 * Purpose: Allows clients to offer customisable keymaps
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_KEYMAP_H
#define APPENGINE_KEYMAP_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#define T keymap_t

typedef int keymap_action;

typedef struct
{
  const char   *name;
  keymap_action action;
}
keymap_name_to_action;

typedef struct
{
  const char                  *name;
  const keymap_name_to_action *mappings;
  int                          nmappings;
}
keymap_section;

typedef struct T T;

/* Uses the specified mapping to parse the keymap <filename> creating a
 * mapping between key numbers and operation codes. */
result_t keymap_create(const char           *filename,
                       const keymap_section *sections,
                       int                   nsections,
                       T                   **keymap);

/* Returns the operation associated with the specified key number. */
int keymap_get_action(T *keymap, int section, wimp_key_no key_no);

void keymap_destroy(T *keymap);

#undef T

#endif /* APPENGINE_KEYMAP_H */
