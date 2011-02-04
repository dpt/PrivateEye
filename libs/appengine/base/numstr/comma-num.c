
#include "oslib/os.h"
#include "oslib/territory.h"

#include "appengine/base/numstr.h"

void comma_number(int num, char *buf, int bufsz)
{
  char *psep;
  char  sep;
  char *end;
  char *p;

  psep = territory_read_string_symbols(territory_CURRENT,
                                       territory_SYMBOL_GROUP_SEPARATOR);

  sep = *psep; /* this assumes that the separator is a single char */

  end = os_convert_spaced_cardinal4(num, buf, bufsz);
  for (p = buf; p < end; p++)
    if (*p == ' ')
      *p = sep;
  *end = '\0';
}
