/* --------------------------------------------------------------------------
 *    Name: open-dir.c
 * Purpose: filer_open_dir
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/filer.h"
#include "oslib/wimp.h"

#include "appengine/base/strings.h"

#include "appengine/wimp/filer.h"

void filer_open_dir(const char *filename)
{
  const char             *path;
  filer_message_open_dir *open_dir;
  wimp_message            message;

  path = str_branch(filename);

  open_dir        = (filer_message_open_dir *) &message.data;
  open_dir->fs_no = fileswitch_FS_NUMBER_NONE;
  open_dir->flags = 0;
  str_n_cpy(open_dir->dir_name, path, sizeof(open_dir->dir_name));

  message.action   = message_FILER_OPEN_DIR;
  message.your_ref = 0;
  message.size     = wimp_SIZEOF_MESSAGE_HEADER((
                       offsetof(filer_message_open_dir, dir_name) +
                       str_len(open_dir->dir_name) + 1 + 3) & ~3);

  wimp_send_message(wimp_USER_MESSAGE, &message, wimp_BROADCAST);
}
