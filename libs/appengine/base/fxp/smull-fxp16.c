
#include "appengine/base/fxp.h"

#define T  int
#define LL long long

T smull_fxp16(T x, T y)
{
  return (T) (((LL) x * (LL) y) >> 16);
}
