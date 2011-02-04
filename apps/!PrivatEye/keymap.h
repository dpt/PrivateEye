/* --------------------------------------------------------------------------
 *    Name: keymap.h
 * Purpose: Keymap
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_KEYMAP_H
#define VIEWER_KEYMAP_H

#include "oslib/wimp.h"

#include "appengine/base/errors.h"

typedef enum viewer_keymap_section
{
  viewer_keymap_SECTION_COMMON,
#ifdef EYE_TAGS
  viewer_keymap_SECTION_TAG_CLOUD,
#endif
#ifdef EYE_THUMBVIEW
  viewer_keymap_SECTION_THUMBVIEW,
#endif
  viewer_keymap_SECTION_VIEWER,
  viewer_keymap_SECTION__LIMIT,
}
viewer_keymap_section;

error viewer_keymap_init(void);
void viewer_keymap_fin(void);
int viewer_keymap_op(viewer_keymap_section section, wimp_key_no key_no);

#endif /* VIEWER_KEYMAP_H */
