/* $Id: smull-fxp16.c,v 1.1 2010-01-10 21:54:23 dpt Exp $ */

#include "appengine/base/fxp.h"

#define T  int
#define LL long long

T smull_fxp16(T x, T y)
{
  return (T) (((LL) x * (LL) y) >> 16);
}
