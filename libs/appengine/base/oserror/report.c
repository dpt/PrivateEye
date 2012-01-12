
#include "swis.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/os.h"

#include "appengine/base/appengine.h"
#include "appengine/base/messages.h"

#include "appengine/base/oserror.h"

/*
 * oserror_report
 *
 * Open an error box with an 'OK' (or 'Continue') button.
 *
 */

void oserror_report(int errnum, const char *format_token, ...)
{
  os_error e;
  va_list  argp;
  char     icon[13]; /* twelve characters plus terminator */

  /* Fill in the os_error block */
  e.errnum = errnum;
  va_start(argp, format_token);
  vsprintf(e.errmess, message(format_token), argp);
  va_end(argp);

  /* Can't use message() twice in parameters, so get a copy first */
  strncpy(icon, message("icon"), sizeof(icon));
  icon[12] = '\0'; /* ensure termination (safety) */

  EC(_swix(Wimp_ReportError, _INR(0,5), &e, 0x501, message("task"), icon, NULL /* no sprite area */, NULL /* no extra buttons */));
}
