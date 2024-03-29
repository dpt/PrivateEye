/* --------------------------------------------------------------------------
 *    Name: globals.h
 * Purpose: Global variables
 * ----------------------------------------------------------------------- */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "oslib/os.h"
#include "oslib/wimp.h"

#include "databases/tag-db.h"

#include "appengine/graphics/imagecache.h"
#include "appengine/wimp/dialogue.h"

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
#ifdef EYE_THUMBVIEW
  wimp_w           thumbview_w;
#endif
#ifdef EYE_CANVAS
  wimp_w           canvas_w;
#endif

  viewer_t        *current_viewer;

  wimp_menu       *image_m;
#ifdef EYE_THUMBVIEW
  wimp_menu       *thumbview_m;
#endif
#ifdef EYE_CANVAS
  wimp_menu       *canvas_m;
#endif

  eye_choices      choices, proposed_choices;

  /* If non-NULL we own the clipboard. */
  viewer_t        *clipboard_viewer;

  wimp_t           task_handle;

  os_box           scaling_drag_bbox;

  wimp_version_no  wimp_version;

  viewer_t        *dragging_viewer;

  imagecache_t    *cache;
}
GLOBALS;

#endif /* GLOBALS_H */
