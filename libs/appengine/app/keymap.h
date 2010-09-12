/* --------------------------------------------------------------------------
 *    Name: keymap.h
 * Purpose: Allows clients to offer customisable keymaps
 *  Author: David Thomas
 * Version: $Id: keymap.h,v 1.2 2010-01-13 18:41:09 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_KEYMAP_H
#define APPENGINE_KEYMAP_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

#define T keymap_t

typedef int keymap_action;

typedef struct
{
  const char                   *name;
  keymap_action                 action;
}
keymap_name_to_action;

typedef struct
{
  const char                   *name;
  const keymap_name_to_action  *mappings;
  int                           nmappings;
}
keymap_section;

typedef struct T T;

/* Uses the specified mapping to parse the keymap <filename> creating a
 * maping between key numbers and operation codes. */
error keymap__create(const char           *filename,
                     const keymap_section *sections,
                     int                   nsections,
                     T                   **keymap);

/* Returns the operation associated with the specified key number. */
int keymap__action(T *keymap, int section, wimp_key_no key_no);

void keymap__destroy(T *keymap);

#undef T

#endif /* APPENGINE_KEYMAP_H */
