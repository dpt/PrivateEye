
#include <stdlib.h>
#include <string.h>

#include "oslib/os.h"

#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"

#include "appengine/base/errors.h"

/* FIXME: Unsure if specifying '1' as the error block's error number is the
 * correct thing to do. */

void error__fatal(const char *token)
{
  os_error e;

  e.errnum = 1;
  strncpy(e.errmess, message(token), sizeof(e.errmess));

  oserror__report_block(&e);

  exit(EXIT_FAILURE);
}

void error__fatal1(const char *token, const char *parameter1)
{
  os_error e;

  e.errnum = 1;
  strncpy(e.errmess, message1(token, parameter1), sizeof(e.errmess));

  oserror__report_block(&e);

  exit(EXIT_FAILURE);
}

void error__fatal_oom(void)
{
  error__fatal("NoMem"); /* uses the global messages */
}
