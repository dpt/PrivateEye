
#include "kernel.h"
#include "swis.h"

#include <stddef.h>

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"
#include "appengine/net/internet.h"

const char *mime_fromfiletype(int filetype)
{
  static char buffer[256];

  if (EC(_swix(0x50b00 /* MimeMap_Translate */,
               _INR(0, 3),
               0,
               filetype,
               2,
               buffer)) != NULL)
    return "application/octet-stream";

  return buffer;
}
