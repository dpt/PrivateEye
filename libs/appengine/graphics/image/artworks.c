/* --------------------------------------------------------------------------
 *    Name: artworks.c
 * Purpose: ArtWorks module
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/draw.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/graphics/artworks.h"
#include "appengine/base/oserror.h"
#include "appengine/base/messages.h"

#include "appengine/graphics/awrender.h"
#include "generic.h"

#include "artworks.h"

static int artworks_load(const image_choices *choices, image_t *image)
{
  static osbool   have_renderer = FALSE;

  os_error       *e;
  int             file_size;
  artworks_handle handle;
  int             border_size;
  os_box         *bbox;

  if (have_renderer == FALSE)
  {
    /* Check the ArtWorks renderer is present. */
    if (xos_cli("LoadArtWorksModules")) /* no _kernel_system in GCC */
    {
      oserror_report(1, "error.no.artworks");
      return TRUE; /* failure */
    }
    else
    {
      have_renderer = TRUE;
    }
  }

  osfile_read_no_path(image->file_name,
                     &image->source.load,
                     &image->source.exec,
             (int *) &image->source.file_size,
                      NULL);

  file_size = image->source.file_size;

  if (flex_alloc((flex_ptr) &image->image, file_size) == 0)
  {
    oserror_report(1, "error.no.mem");
    return TRUE; /* failure */
  }

  if (flex_alloc((flex_ptr) &image->details.artworks.workspace,
                  awrender_DefaultWorkSpace) == 0)
  {
    oserror_report(1, "error.no.mem");
    flex_free((flex_ptr) &image->image);
    return TRUE; /* failure */
  }

  osfile_load_stamped_no_path(image->file_name,
                              image->image,
                              NULL,
                              NULL,
                              NULL,
                              NULL);

  // transfer ownership of blocks here?
  handle.resizable_block = &image->image;
  handle.fixed_block     = (void *) -1;

  e = (os_error *) awrender_file_init(image->image,
          (awrender_callback_handler) artworks_callback,
                                      file_size,
                                     &handle);
  if (e != NULL)
  {
    flex_free((flex_ptr) &image->details.artworks.workspace);
    flex_free((flex_ptr) &image->image);
    oserror_report_block(e);
    return TRUE; /* failure */
  }

  bbox = &image->display.dims.vc.box;

  awrender_doc_bounds((awrender_doc) image->image, bbox);

  /* Add a border */
  border_size = choices->artworks.border * draw_OS_UNIT;
  bbox->x0 -= border_size;
  bbox->y0 -= border_size;
  bbox->x1 += border_size;
  bbox->y1 += border_size;

  image->flags = image_FLAG_VECTOR;

  image->display.file_size = file_size;
  image->display.file_type = artworks_FILE_TYPE;

  image->source.dims.vc = image->display.dims.vc;
  /* image->source.load, exec, file_size done */
  image->source.file_type = artworks_FILE_TYPE;

  image->scale.min = 1;
  image->scale.max = 4000;

  return FALSE; /* success */
}

static int artworks_unload(image_t *image)
{
  flex_free((flex_ptr) &image->details.artworks.workspace);
  flex_free((flex_ptr) &image->image);

  return FALSE; /* success */
}

void artworks_export_methods(const image_choices *choices, image_t *image)
{
  static const image_methods methods =
  {
    artworks_load,
    generic_save,
    artworks_unload,
    NULL,
    NULL,
    NULL,
    NULL
  };

  NOT_USED(choices);

  image->methods = methods;
}
