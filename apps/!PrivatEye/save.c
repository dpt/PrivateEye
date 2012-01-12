/* --------------------------------------------------------------------------
 *    Name: save.c
 * Purpose: Viewer save dialogue handler
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/types.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/dialogues/save.h"

#include "globals.h"
#include "viewer.h"

#include "save.h"

/* ----------------------------------------------------------------------- */

dialogue_t *viewer_savedlg;

/* ----------------------------------------------------------------------- */

static void viewer_savedlg_fillout(dialogue_t *d, void *arg)
{
  viewer_t *viewer;
  image_t  *image;

  NOT_USED(arg);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  save_set_file_name(d, image->file_name);
  save_set_file_type(d, image->display.file_type);
}

/* Called on 'Save' button clicks, but not on drag saves. */
static void viewer_savedlg_handler(dialogue_t *d, const char *file_name)
{
  viewer_t *viewer;

  NOT_USED(d);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  viewer_save(viewer, file_name);
}

/* ----------------------------------------------------------------------- */

error viewer_savedlg_init(void)
{
  viewer_savedlg = save_create();
  if (viewer_savedlg == NULL)
    return error_OOM;

  dialogue_set_fillout_handler(viewer_savedlg, viewer_savedlg_fillout, NULL);
  save_set_save_handler(viewer_savedlg, viewer_savedlg_handler);

  return error_OK;
}

void viewer_savedlg_fin(void)
{
  save_destroy(viewer_savedlg);
}
