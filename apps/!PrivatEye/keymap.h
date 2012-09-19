/* --------------------------------------------------------------------------
 *    Name: keymap.h
 * Purpose: Keymap
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_KEYMAP_H
#define VIEWER_KEYMAP_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"
#include "appengine/app/keymap.h"

/* ---------------------------------------------------------------------- */

/* common key defns */
/* numbered 1024+ to avoid a clash with other values */
enum
{
  Close = 1024,
  Help,
};

/* ---------------------------------------------------------------------- */

typedef int viewer_keymap_id;

error viewer_keymap_init(void);
void viewer_keymap_fin(void);

error viewer_keymap_add(const char                  *name,
                        const keymap_name_to_action *mappings,
                        int                          nmappings,
                        viewer_keymap_id            *id);

int viewer_keymap_op(viewer_keymap_id id, wimp_key_no key_no);

#endif /* VIEWER_KEYMAP_H */
