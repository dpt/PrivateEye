/* --------------------------------------------------------------------------
 *    Name: sprite.c
 * Purpose: Sprite module
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/osfile.h"

#include "appengine/base/oserror.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"

#include "bitmap.h"

#include "sprite.h"

static result_t sprite_populate_info(image_t *image, os_mode mode)
{
  result_t rc;
  char     buf[64];

  sprite_describe_mode(mode, buf, sizeof(buf));

  rc = image_set_info(image, image_INFO_FORMAT, buf);
  if (rc)
    return rc;

  return result_OK;
}

static int sprite_load(image_choices *choices, image_t *image)
{
  result_t           rc;
  int                file_size;
  int                log2bpp;
  osspriteop_area   *area;
  osspriteop_header *header;
  os_error          *e;
  osbool             has_mask;
  os_mode            mode;
  osspriteop_mode_word mode_word;

  NOT_USED(choices);

  osfile_read_no_path(image->file_name,
                     &image->source.load,
                     &image->source.exec,
             (int *) &image->source.file_size,
                      NULL /* attr */);

  file_size = image->source.file_size + sizeof(area->size);

  if (flex_alloc((flex_ptr) &area, file_size) == 0)
    goto NoMem;

  area->size = file_size;
  area->first = 16;
  osspriteop_clear_sprites(osspriteop_PTR, area);

  e = EC(xosspriteop_load_sprite_file(osspriteop_PTR,
                                      area,
                                      image->file_name));

  if (e == NULL)
    e = EC(xosspriteop_verify_area(osspriteop_PTR, area));

  if (e)
  {
    flex_free((flex_ptr) &area);
    oserror_report_block(e);
    return TRUE; /* failure */
  }

  header = sprite_select(area, 0);

  osspriteop_read_sprite_info(osspriteop_PTR,
                              area,
              (osspriteop_id) header,
                             &image->display.dims.bm.width,
                             &image->display.dims.bm.height,
                             &has_mask,
                             &mode);
  mode_word = (osspriteop_mode_word) mode;

  image->flags = image_FLAG_COLOUR | image_FLAG_CAN_ROT;

  if (has_mask)
    image->flags |= image_FLAG_HAS_MASK;
  if ((osspriteop_mode_word) mode & osspriteop_ALPHA_MASK)
    image->flags |= image_FLAG_HAS_ALPHA_MASK;

  switch (sprite_type(mode_word))
  {
  case osspriteop_TYPE32BPP:
    image->flags |= image_FLAG_CAN_HIST;
    if (sprite_has_alpha(header))
      image->flags |= image_FLAG_HAS_ALPHA;
    break;
  case osspriteop_TYPE_CMYK:
    image->flags |= image_FLAG_CMYK;
    break;
  }

  read_mode_vars(mode, &image->display.dims.bm.xeig,
                       &image->display.dims.bm.yeig,
                       &log2bpp);
  
  /* Pass mode separately since it's not setup yet. */
  rc = sprite_populate_info(image, mode);
  if (rc)
  {
    flex_free((flex_ptr) &area);
    result_report(rc);
    return TRUE; /* failure */
  }

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &area);

  image->display.dims.bm.xdpi = os_INCH >> image->display.dims.bm.xeig;
  image->display.dims.bm.ydpi = os_INCH >> image->display.dims.bm.yeig;
  image->display.dims.bm.bpp  = 1 << log2bpp;
  image->display.file_size    = file_size;
  image->display.file_type    = osfile_TYPE_SPRITE;

  image->source.dims = image->display.dims;
  /* load, exec, file_size set */
  image->source.file_type = osfile_TYPE_SPRITE;

  image->scale.min = 1;
  image->scale.max = 32767;

  image->details.sprite.mode = mode;

  return FALSE; /* success */


NoMem:

  oserror_report(0, "error.no.mem");

  return TRUE; /* failure */
}

void sprite_export_methods(image_choices *choices, image_t *image)
{
  static const image_methods methods =
  {
    sprite_load,
    bitmap_save,
    bitmap_unload,
    bitmap_histogram,
    bitmap_rotate,
    NULL,
    NULL
  };

  NOT_USED(choices);

  image->methods = methods;
}
