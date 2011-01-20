/* --------------------------------------------------------------------------
 *    Name: errors.c
 * Purpose: Errors
 * Version: $Id: errors.c,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/base/oserror.h"

#include "appengine/base/errors.h"

void error__report(error err)
{
  static const struct
  {
    error       er;
    const char *token;
  }
  map[] =
  {
    { error_OOM,                         "error.no.mem"           },
    { error_PRIVATEEYE_VIEWER_NOT_FOUND, "error.viewer.not.found" },
    { error_KEYMAP_UNKNOWN_ACTION,       "error.keymap.unknown.action" },
  };

  const int   nelems = (int) (sizeof(map) / sizeof(map[0]));
  int         i;
  const char *token;

  if (!err)
    return;

  token = "error.unknown";

  for (i = 0; i < nelems; i++)
  {
    if (map[i].er == err)
    {
      token = map[i].token;
      break;
    }
  }

  oserror__report(1, token, err);
}
