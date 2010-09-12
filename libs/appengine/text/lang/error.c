/* $Id: error.c,v 1.2 2009-02-05 23:49:23 dpt Exp $ */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void lerror(const char *fmt, ...)
{
  va_list arg;

  va_start(arg, fmt);

  vprintf(fmt, arg);
  printf("\n");

  va_end(arg);

  exit(EXIT_FAILURE);
}
