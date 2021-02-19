/* --------------------------------------------------------------------------
 *    Name: recognise.c
 * Purpose: Recognise a file based on its type
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osgbpb.h"

#include "appengine/base/bsearch.h"
#include "appengine/base/oserror.h"

#include "gif.h"
#include "png.h"
#include "artworks.h"

#include "appengine/graphics/image.h"

int image_recognise(wimp_message *message)
{
  os_error *e;
  os_fw     f;
  byte      buf[4];
  size_t    bufsz;
  bits      file_type;

  /* read part of the file */
  e = EC(xosfind_openinw(osfind_NO_PATH         |
                         osfind_ERROR_IF_ABSENT |
                         osfind_ERROR_IF_DIR,
                         message->data.data_xfer.file_name,
                         NULL,
                        &f));
  if (e)
  {
    oserror_report_block(e);
    return 1; /* failure */
  }

  /* bytes in buffer */
  bufsz = sizeof(buf) - osgbpb_readw(f, buf, sizeof(buf));

  osfind_closew(f);

  file_type = 0;

  if (bufsz >= sizeof(buf))
  {
    static const struct
    {
      unsigned int magic;
      bits         file_type;
    }
    map[] =
    {
      { 0x21706f54, /* "Top!"    */ artworks_FILE_TYPE },
      { 0x38464947, /* "GIF8"    */ gif_FILE_TYPE      },
      { 0x474e5089, /* "PNG\x89" */ png_FILE_TYPE      },
      { 0x77617244, /* "Draw"    */ osfile_TYPE_DRAW   },
      { 0xe0ffd8ff, /* "yoya"    */ jpeg_FILE_TYPE     }
    };

    const size_t stride = sizeof(map[0]);
    const size_t nelems = sizeof(map) / stride;

    int          i;
    unsigned int magic;

    magic = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    i = bsearch_uint(&map[0].magic, nelems, stride, magic);
    if (i >= 0)
      file_type = map[i].file_type;
  }

  if (file_type == 0)
    return 1; /* failed to recognise */

  message->data.data_xfer.file_type = file_type;

  (void) xosfile_set_type(message->data.data_xfer.file_name,
                          message->data.data_xfer.file_type);

  return 0; /* success */
}
