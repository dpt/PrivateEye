/* --------------------------------------------------------------------------
 *    Name: errors.c
 * Purpose: Errors
 * ----------------------------------------------------------------------- */

#include <stdarg.h>
#include <stdio.h>

#include "appengine/base/messages.h"
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

  if (err == result_OS)
  {
    oserror_report_block(oserror_last());
    return;
  }

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

/* ----------------------------------------------------------------------- */

static const os_error *stashed_os_error;

result_t oserror_stash(const os_error *e)
{
  stashed_os_error = e;

  return result_OS;
}

result_t oserror_build(int errnum, const char *format_token, ...)
{
  static os_error e;

  va_list argp;

  /* Fill in the os_error block */
  e.errnum = errnum;
  va_start(argp, format_token);
  vsprintf(e.errmess, message(format_token), argp);
  va_end(argp);

  oserror_stash(&e);

  return result_OS;
}

const os_error *oserror_last(void)
{
  return stashed_os_error;
}

void oserror_clear(void)
{
  stashed_os_error = NULL;
}
