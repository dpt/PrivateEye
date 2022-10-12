/* --------------------------------------------------------------------------
 *    Name: getmodver.c
 * Purpose: Get module version number
 * ----------------------------------------------------------------------- */

#include <string.h>

#include "oslib/os.h"
#include "oslib/osmodule.h"

#include "appengine/base/os.h"

int get_module_version(const char *module_name)
{
  os_error *e;
  int       module_no;
  int       section;
  char     *this_module_name;
  int       bcd_version;

  module_no = 0;
  section   = -1;
  for (;;)
  {
    e = xosmodule_enumerate_rom_with_info(module_no,
                                          section,
                                         &module_no,
                                         &section,
                                         &this_module_name,
                                          NULL, /* status */
                                          NULL, /* chunk_no */
                                         &bcd_version);
    if (e)
      return -1; /* unknown */

    if (strcmp(this_module_name, module_name) == 0)
      return bcd_version;
  }
}


