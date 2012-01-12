
#include <stdlib.h>
#include <string.h>

#include "oslib/os.h"

#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"

#include "appengine/base/errors.h"

/* FIXME: Unsure if specifying '1' as the error block's error number is the
 * correct thing to do. */

void error_fatal(const char *token)
{
  os_error e;

  e.errnum = 1;
  strncpy(e.errmess, message(token), sizeof(e.errmess));

  oserror_report_block(&e);

  exit(EXIT_FAILURE);
}

void error_fatal1(const char *token, const char *parameter1)
{
  os_error e;

  e.errnum = 1;
  strncpy(e.errmess, message1(token, parameter1), sizeof(e.errmess));

  oserror_report_block(&e);

  exit(EXIT_FAILURE);
}

void error_fatal_oom(void)
{
  error_fatal("NoMem"); /* uses the global messages */
}
