/* --------------------------------------------------------------------------
 *    Name: loaders.c
 * Purpose: Loaders
 * ----------------------------------------------------------------------- */

#include <string.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/jpeg.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osgbpb.h"

#include "appengine/base/bsearch.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"
#include "appengine/base/utils.h"

#include "artworks.h"
#include "bitmap.h"
#include "drawfile.h"
#include "gif.h"
#include "jpeg.h"
#include "png.h"
#include "sprite.h"

/* This implements stuff in _both_ the following files. */

#include "appengine/graphics/image.h"
#include "formats.h"

static const struct
{
  bits    file_type;
  void  (*export_methods)(image_choices *, image_t *);
  osbool *load;
}
TypesData_map[] =
{
  { /* 0x695 */ gif_FILE_TYPE,      gif_export_methods      },
  { /* 0xaff */ osfile_TYPE_DRAW,   drawfile_export_methods },
  { /* 0xb60 */ png_FILE_TYPE,      png_export_methods      },
  { /* 0xc85 */ jpeg_FILE_TYPE,     jpeg_export_methods     },
  { /* 0xd94 */ artworks_FILE_TYPE, artworks_export_methods },
  { /* 0xff9 */ osfile_TYPE_SPRITE, sprite_export_methods   }
};

static const size_t TypesData_stride = sizeof(TypesData_map[0]);
static const size_t TypesData_nelems = sizeof(TypesData_map) / sizeof(TypesData_map[0]);

static int filetype_to_index(bits file_type)
{
  return bsearch_uint(&TypesData_map[0].file_type,
                       TypesData_nelems,
                       TypesData_stride,
                       file_type);
}

osbool image_is_loadable(bits file_type)
{
  return filetype_to_index(file_type) >= 0;
}

osbool loader_export_methods(image_choices *choices,
                             image_t       *image,
                             bits           file_type)
{
  int i;

  i = filetype_to_index(file_type);
  if (i < 0)
    return TRUE; /* failure */

  TypesData_map[i].export_methods(choices, image);

  return FALSE; /* success */
}
