
#include <stdio.h>

#include "oslib/types.h"
#include "oslib/osgbpb.h"

#include "appengine/types.h"
#include "appengine/io/filing.h"

static result_t callback(const char          *obj_name,
                         osgbpb_info_stamped *info,
                         void                *context)
{
  NOT_USED(info);
  NOT_USED(context);

  printf("%s\n", obj_name);

  return result_OK;
}

int filing_test(void)
{
  result_t err;

  printf("test: filing: dirscan\n");

  err = dirscan("@",
                callback,
                dirscan_FILES | dirscan_DIRECTORIES,
                NULL);

  return 0;
}
