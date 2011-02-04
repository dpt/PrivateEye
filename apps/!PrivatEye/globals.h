/* --------------------------------------------------------------------------
 *    Name: globals.h
 * Purpose: Global variables
 * ----------------------------------------------------------------------- */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/databases/tag-db.h"

#include "privateeye.h"
#include "viewer.h"

enum
{
  Flag_Quit = 1 << 0
};

typedef unsigned int Flags;

/* PrivateEye's global variables are all grouped together to allow the
 * compiler to use "base pointer optimisation". */

extern struct PrivateEyeGlobals
{
  Flags            flags;

  wimp_w           display_w;
  wimp_w           thumbview_w;

  wimp_w           effects_w;
  wimp_w           effects_add_w;
  wimp_w           effects_crv_w;
  dialogue_t       effects_blr_d;

  wimp_w           current_display_w;

  wimp_menu       *iconbar_m;
  wimp_menu       *image_m;
  wimp_menu       *thumbview_m;

  eye_choices      choices, proposed_choices;

  /* If non-NULL we own the clipboard. */
  viewer_t        *clipboard_viewer;

  wimp_t           task_handle;

  os_box           scaling_drag_bbox;

  wimp_version_no  wimp_version;

  viewer_t        *dragging_viewer;
}
GLOBALS;

#endif /* GLOBALS_H */
