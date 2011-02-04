/* --------------------------------------------------------------------------
 *    Name: next.c
 * Purpose: Bit vectors
 * ----------------------------------------------------------------------- */

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/bitvec.h"

#include "impl.h"

int bitvec__next(const bitvec_t *v, int n)
{
  int hi,lo;

  if (!v->vec)
    return -1; /* no vec allocated */

  if (n >= 0)
  {
    hi = n >> 5;
    lo = n & 0x1f;

    if (lo == 0x1f) /* at word boundary: skip first step */
    {
      hi++;
      lo = -1; /* -1 => no mask */
    }
  }
  else
  {
    hi = 0;
    lo = -1;
  }

  for (; hi < v->length; hi++)
  {
    unsigned int word;
    unsigned int bits;

    word = v->vec[hi];
    if (word == 0)
      continue; /* no bits set */

    if (lo >= 0)
      word &= ~((1 << (lo + 1)) - 1);

    bits = lsb(word);
    if (bits)
      return (hi << 5) + ctz(bits);

    lo = -1; /* don't mask the next words */
  }

  return -1; /* ran out of words */
}
