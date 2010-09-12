/* --------------------------------------------------------------------------
 *    Name: generic.c
 * Purpose: Generic viewer methods
 * Version: $Id: generic.c,v 1.2 2009-05-18 22:07:50 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "oslib/os.h"
#include "oslib/osfile.h"

#include "appengine/base/oserror.h"
#include "appengine/graphics/image.h"

#include "generic.h"

int generic_save(image_choices *choices, image *image, const char *file_name)
{
  os_error *e;

  NOT_USED(choices);

  e = EC(xosfile_save_stamped(file_name,
                              image->display.file_type,
                              image->image,
                     (byte *) image->image + image->display.file_size));
  if (e)
  {
    oserror__report_block(e);
    return TRUE; /* failure */
  }

  image->flags &= ~image_FLAG_MODIFIED;

  return FALSE; /* success */
}
