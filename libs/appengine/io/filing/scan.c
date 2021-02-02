
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/osgbpb.h"

#include "appengine/base/errors.h"

#include "appengine/io/filing.h"

#define BUFFER_SIZE 4096

result_t dirscan(const char       *dir_name,
              dirscan_callback *callback,
              dirscan_flags     flags,
              void             *opaque)
{
  result_t                err;
  osgbpb_info_stamped *info     = NULL;
  int                  dir_name_len;
  int                  namebuf_len;
  char                *namebuf  = NULL;
  char                *leaf_ptr;
  int                  context;
  int                  nread;

  info = malloc(BUFFER_SIZE);
  if (info == NULL)
  {
    err = result_OOM;
    goto Failure;
  }

  dir_name_len = strlen(dir_name);

  namebuf_len = dir_name_len + 1 + 10 + 1;
  namebuf = malloc(namebuf_len);
  if (namebuf == NULL)
  {
    err = result_OOM;
    goto Failure;
  }

  memcpy(namebuf, dir_name, dir_name_len);

  leaf_ptr = namebuf + dir_name_len;
  *leaf_ptr++ = '.';

  context = 0;
  do
  {
    osgbpb_info_stamped *p;

    context = osgbpb_dir_entries_info_stamped(dir_name,
                 (osgbpb_info_stamped_list *) info,
                                              INT_MAX,
                                              context,
                                              BUFFER_SIZE,
                                              NULL, /* "*" */
                                             &nread);
    p = info;

    while (nread--)
    {
      int leaf_name_len;
      int new_size;

      /* construct the full pathname */

      leaf_name_len = strlen(p->name);

      new_size = leaf_ptr - namebuf + leaf_name_len + 1;
      if (new_size > namebuf_len)
      {
        /* need to grow name buffer */

        char *new_namebuf;

        /* printf("reallocing to %d\n", new_size); */

        new_namebuf = realloc(namebuf, new_size);
        if (new_namebuf == NULL)
        {
          err = result_OOM;
          goto Failure;
        }

        leaf_ptr   += new_namebuf - namebuf;
        namebuf     = new_namebuf;
        namebuf_len = new_size;
      }

      memcpy(leaf_ptr, p->name, leaf_name_len + 1);

      /* deal with object */

      switch (p->obj_type)
      {
      case fileswitch_IS_DIR:

        if (flags & dirscan_DIRECTORIES)
        {
          err = callback(namebuf, p, opaque);
          if (err)
            goto Failure;
        }

        if (flags & dirscan_RECURSE)
        {
          err = dirscan(namebuf, callback, flags, opaque);
          if (err)
            goto Failure;
        }

        break;

      case fileswitch_IS_FILE:
      case fileswitch_IS_IMAGE:

        if (flags & dirscan_FILES)
        {
          err = callback(namebuf, p, opaque);
          if (err)
            goto Failure;
        }

        break;

      default:
        assert("Unknown type of object encountered." == NULL);
      }

      /* move buffer pointer forward */

      p = (osgbpb_info_stamped *) ((char *) p + ((offsetof(osgbpb_info_stamped, name) + leaf_name_len + 1 + 3) & ~ 3));
    }
  }
  while (context != -1);

  err = result_OK;

Failure:

  free(namebuf);

  free(info);

  return err;
}
