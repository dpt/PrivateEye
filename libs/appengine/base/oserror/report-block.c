
#include "swis.h"

#include <string.h>

#include "fortify/fortify.h"

/* #include "MemCheck:MemCheck.h" */

#include "oslib/os.h"

#include "appengine/base/appengine.h"
#include "appengine/base/messages.h"

#include "appengine/base/oserror.h"

/*
 * oserror__report_block
 *
 * Open an error box with an 'OK' (or 'Continue') button, given a
 * os_error block.
 *
 */

void oserror__report_block(os_error *error)
{
  os_error e;
  char     icon[13]; /* twelve characters plus terminator */

  /* MemCheck_RegisterMiscBlock(error, sizeof(os_error)); */

  /* Fill in the os_error block */
  e.errnum = error->errnum & ~ (1u << 31); /* Stop Wimp_ReportError being clever */
  strncpy(e.errmess, error->errmess, sizeof(error->errmess));

  /* Can't use message() twice in parameters, so get a copy first */
  strncpy(icon, message("icon"), sizeof(icon));
  icon[12] = '\0'; /* ensure termination (safety) */

  EC(_swix(Wimp_ReportError, _INR(0,5), &e, 0x501, message("task"), icon, NULL /* no sprite area */, NULL /* no extra buttons */));

  /* MemCheck_UnRegisterMiscBlock(error); */
}
