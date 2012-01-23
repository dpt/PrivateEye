/* delete.c */

#include <assert.h>
#include <stdlib.h>

#include "oslib/osfile.h"

#include "appengine/databases/pickle.h"

void pickle_delete(const char *filename)
{
  assert(filename);

  xosfile_delete(filename, NULL, NULL, NULL, NULL, NULL);
}
