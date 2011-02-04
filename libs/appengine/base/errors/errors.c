/* --------------------------------------------------------------------------
 *    Name: errors.c
 * Purpose: Errors
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
    { error_KEYMAP_UNKNOWN_SECTION,      "error.keymap.unknown.section" },
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
