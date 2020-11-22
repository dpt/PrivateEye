/* --------------------------------------------------------------------------
 *    Name: filetype-name.c
 * Purpose: File type to Sprite name
 * ----------------------------------------------------------------------- */

#include <stdio.h>

#include "oslib/types.h"
#include "oslib/osfile.h"
#include "oslib/osspriteop.h"

#include "appengine/wimp/icon.h"

void file_type_to_sprite_name(bits file_type, char name[osspriteop_NAME_LIMIT])
{
  const char *fmt;

  switch (file_type)
  {
  case osfile_TYPE_UNTYPED:
    fmt = "file_xxx";
    break;

  case osfile_TYPE_DIR:
    fmt = "directory";
    break;

  case osfile_TYPE_APPLICATION:
    fmt = "application";
    break;

  default:
    fmt = "file_%03x";
    break;
  }

  sprintf(name, fmt, file_type);
}
