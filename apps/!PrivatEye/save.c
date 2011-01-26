/* --------------------------------------------------------------------------
 *    Name: save.c
 * Purpose: Save
 * Version: $Id: save.c,v 1.18 2009-05-21 22:43:42 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/types.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/dialogues/save.h"

#include "globals.h"
#include "viewer.h"

#include "save.h"

/* ----------------------------------------------------------------------- */

dialogue_t *save;

/* ----------------------------------------------------------------------- */

static void save_fillout(dialogue_t *d, void *arg)
{
  viewer_t *viewer;
  image    *image;

  NOT_USED(arg);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  save__set_file_name(d, image->file_name);
  save__set_file_type(d, image->display.file_type);
}

/* Called on 'Save' button clicks, but not on drag saves. */
static void save_handler(dialogue_t *d, const char *file_name)
{
  viewer_t *viewer;

  NOT_USED(d);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  viewer_save(viewer, file_name);
}

/* ----------------------------------------------------------------------- */

error viewer_save_init(void)
{
  save = save__create();
  if (save == NULL)
    return error_OOM;

  dialogue__set_fillout_handler(save, save_fillout, NULL);
  save__set_save_handler(save, save_handler);

  return error_OK;
}

void viewer_save_fin(void)
{
  save__destroy(save);
}
