/* $Id: set-double.c,v 1.1 2009-04-29 23:32:01 dpt Exp $ */

#include <assert.h>
#include <stdio.h>

#include "oslib/wimp.h"

#include "appengine/wimp/icon.h"

void icon_set_double(wimp_w w, wimp_i i, double value, int places)
{
  char format[5];  /* "%.9f" + NUL */
  char string[22];

  assert(places <= 9);

  format[0] = '%';
  format[1] = '.';
  format[2] = '0' + places;
  format[3] = 'f';
  format[4] = 0;

  sprintf(string, format, value);
  icon_set_text(w, i, string);
}
