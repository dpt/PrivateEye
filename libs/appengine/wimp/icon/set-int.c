
#include <stdio.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_set_int(wimp_w w, wimp_i i, int value)
{
  char string[12]; /* enough for INT_MIN */

  sprintf(string, "%d", value);
  icon_set_text(w, i, string);
}
