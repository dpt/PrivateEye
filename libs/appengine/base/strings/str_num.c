/* $Id: str_num.c,v 1.1 2009-05-18 22:07:49 dpt Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "appengine/base/appengine.h"
#include "appengine/base/strings.h"

const char *str_num(int number)
{
  static char buffer[11];

  sprintf(buffer, "%d", number);

  return buffer;
}
