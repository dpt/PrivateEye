/* --------------------------------------------------------------------------
 *    Name: errors.c
 * Purpose: Errors
 * ----------------------------------------------------------------------- */

#include "appengine/base/oserror.h"

#include "appengine/base/errors.h"

void result_report(result_t err)
{
  static const struct
  {
    result_t    er;
    const char *token;
  }
  map[] =
  {
    { result_OOM,                         "error.no.mem"           },
    { result_PRIVATEEYE_VIEWER_NOT_FOUND, "error.viewer.not.found" },
    { result_KEYMAP_UNKNOWN_ACTION,       "error.keymap.unknown.action" },
    { result_KEYMAP_UNKNOWN_SECTION,      "error.keymap.unknown.section" },
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

  oserror_report(1, token, err);
}
