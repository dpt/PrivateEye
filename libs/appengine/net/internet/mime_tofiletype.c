/* $Id: mime_tofiletype.c,v 1.5 2009-05-18 22:07:51 dpt Exp $ */

#include "kernel.h"
#include "swis.h"

#include <stddef.h>

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"
#include "appengine/internet.h"

int mime_tofiletype(const char *mimetype)
{
  int filetype;

  if (EC(_swix(0x50b00 /* MimeMap_Translate */, _INR(0,2)|_OUT(3), 2, mimetype, 0, &filetype)) != NULL)
    return 0xffd; /* data */

  return filetype;
}
