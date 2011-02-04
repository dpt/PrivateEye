
#include <float.h>
#include <stdio.h>
#include <string.h>

#include "oslib/types.h"

#include "appengine/base/numstr.h"

void comma_double(double num, char *buf, int bufsz)
{
  int intnum;

  NOT_USED(bufsz);

  intnum = (int) num;
  comma_number(intnum, buf, sizeof(buf));

  num -= intnum;
  if (num > DBL_EPSILON)
  {
    char buf2[8];

    sprintf(buf2, ".%02d", (int) (num * 100.0));
    strcat(buf, buf2);
  }
}
