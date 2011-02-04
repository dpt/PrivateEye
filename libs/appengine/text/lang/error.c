
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
