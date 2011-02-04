
#include <stdarg.h>
#include <stdio.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_validation_printf(wimp_w w, wimp_i i, const char *fmt, ...)
{
  va_list argp;
  char    string[256]; /* Careful Now */

  va_start(argp, fmt);
  vsprintf(string, fmt, argp);
  va_end(argp);

  icon_set_validation(w, i, string);
}
