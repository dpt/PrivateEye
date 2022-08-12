/* --------------------------------------------------------------------------
 *    Name: save.c
 * Purpose: Viewer save dialogue handler
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/types.h"

#include "appengine/base/messages.h"
#include "appengine/dialogues/save.h"
#include "appengine/wimp/dialogue.h"

#include "globals.h"
#include "viewer.h"

#include "save.h"

/* ----------------------------------------------------------------------- */

dialogue_t *viewer_savedlg;

static viewer_t *saving_viewer;

/* ----------------------------------------------------------------------- */

/* Save dialogue was opened for whatever reason. */
static void viewer_savedlg_fillout(dialogue_t *d, void *opaque)
{
  image_t    *image;
  const char *file_name;

  NOT_USED(opaque);

  saving_viewer = GLOBALS.current_viewer;
  if (saving_viewer == NULL)
    return;

  image = saving_viewer->drawable->image;
  if (image->file_name[0] != '\0')
    file_name = image->file_name;
  else
    file_name = message0("untitled.filename");

  // should merge these set methods together
  save_set_file_name(d, file_name);
  save_set_file_type(d, image->display.file_type);
  save_set_file_size(d, image->display.file_size);
}

/* Called on 'Save' button clicks, but not on drag saves. */
static void viewer_savedlg_handler(dialogue_t *d, const char *file_name)
{
  NOT_USED(d);

  if (saving_viewer)
  {
    viewer_save(saving_viewer, file_name, FALSE);
    viewer_savedlg_completed();
  }
}

/* ----------------------------------------------------------------------- */

viewer_t *viewer_savedlg_get(void)
{
  return saving_viewer;
}

void viewer_savedlg_completed(void)
{
  saving_viewer = NULL;
}

/* ----------------------------------------------------------------------- */

result_t viewer_savedlg_init(void)
{
  viewer_savedlg = save_create();
  if (viewer_savedlg == NULL)
    return result_OOM;

  dialogue_set_fillout_handler(viewer_savedlg, viewer_savedlg_fillout, NULL);
  save_set_save_handler(viewer_savedlg, viewer_savedlg_handler);

  return result_OK;
}

void viewer_savedlg_fin(void)
{
  save_destroy(viewer_savedlg);
}
