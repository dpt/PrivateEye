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

  image = GLOBALS.current_viewer->drawable->image;
  if (image->file_name[0] != '\0')
    file_name = image->file_name;
  else
    file_name = message0("untitled.filename");

  save_set_info(d, file_name,
                   image->display.file_type,
                   image->display.file_size);
}

/* This only catches the case of <F3><Esc>. If the save dialogue is opened as
 * a submenu then the owner will need to call viewer_savedlg_reset(). */
static int viewer_savedlg_message_menus_deleted(wimp_message *message,
                                                void         *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  saving_viewer = NULL;

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* Called on 'Save' button clicks, but not on drag saves. */
static void viewer_savedlg_save_handler(dialogue_t *d, const char *file_name)
{
  NOT_USED(d);

  viewer_save(GLOBALS.current_viewer, file_name, FALSE);
}

/* Called on drag saves with the ref of the DataSave message. */
static void viewer_savedlg_dataxfer_handler(dialogue_t *d, int my_ref)
{
  NOT_USED(d);

  saving_viewer = GLOBALS.current_viewer;
  saving_viewer->drawable->image->last_save_ref = my_ref;
}

/* ----------------------------------------------------------------------- */

viewer_t *viewer_savedlg_get(void)
{
  return saving_viewer;
}

void viewer_savedlg_reset(void)
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
  dialogue_set_menus_deleted_handler(viewer_savedlg,
                                     viewer_savedlg_message_menus_deleted);

  save_set_dataxfer_handler(viewer_savedlg, viewer_savedlg_dataxfer_handler);
  save_set_save_handler(viewer_savedlg, viewer_savedlg_save_handler);

  return result_OK;
}

void viewer_savedlg_fin(void)
{
  save_destroy(viewer_savedlg);
}
