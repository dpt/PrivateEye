
#include <stdio.h>

#include "oslib/os.h"

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"

/* #include "MemCheck:MemCheck.h" */

os_error *oserror__check(os_error *e, const char *file, int line)
{
  /* MemCheck_RegisterMiscBlock(e, 256); */

  if (e != NULL)
    fprintf(stderr, "os_error: %s [0x%x] (in %s‘ at line %d).\n",
            e->errmess, e->errnum, file, line);

  /* MemCheck_UnRegisterMiscBlock(e); */

  return e;
}
