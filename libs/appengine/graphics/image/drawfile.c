/* --------------------------------------------------------------------------
 *    Name: drawfile.c
 * Purpose: DrawFile module
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "flex.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/draw.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"
#include "oslib/drawfile.h"

#include "appengine/base/oserror.h"
#include "appengine/base/messages.h"

#include "generic.h"

#include "drawfile.h"

static int drawfile_load(image_choices *choices, image_t *image)
{
  drawfile_diagram *data;
  int               file_size;
  int               border_size;
  os_box           *bbox;

  osfile_read_no_path(image->file_name,
                     &image->source.load,
                     &image->source.exec,
                     &file_size,
                      NULL);

  if (flex_alloc((flex_ptr) &data, file_size) == 0)
  {
    oserror_report(0, "error.no.mem");
    return TRUE; /* failure */
  }

  osfile_load_stamped_no_path(image->file_name,
                     (byte *) data,
                              NULL,
                              NULL,
                              NULL,
                              NULL);

  bbox = &image->display.dims.vc.box;

  /* XXX want to check for errors here! */
  /* (drawfile_bbox_flags) */
  /* (drawfile_diagram *) */
  drawfile_bbox(0, data, file_size, NULL, bbox);

  /* Add a border */
  border_size = choices->drawfile.border * draw_OS_UNIT;
  bbox->x0 -= border_size;
  bbox->y0 -= border_size;
  bbox->x1 += border_size;
  bbox->y1 += border_size;

  image->flags = image_FLAG_VECTOR;

  flex_reanchor((flex_ptr) &image->image, (flex_ptr) &data);

  image->display.file_size = file_size;
  image->display.file_type = osfile_TYPE_DRAW;

  image->source.dims.vc = image->display.dims.vc;
  /* image->source.load, exec done */
  image->source.file_type = osfile_TYPE_DRAW;
  image->source.file_size = file_size;

  image->scale.min = 1;
  image->scale.max = 800;

  return FALSE; /* success */
}

static int drawfile_unload(image_t *image)
{
  flex_free((flex_ptr) &image->image);

  return FALSE; /* success */
}

void drawfile_export_methods(image_choices *choices, image_t *image)
{
  static const image_methods methods =
  {
    drawfile_load,
    generic_save,
    drawfile_unload,
    NULL,
    NULL,
    NULL
  };

  NOT_USED(choices);

  image->methods = methods;
}
