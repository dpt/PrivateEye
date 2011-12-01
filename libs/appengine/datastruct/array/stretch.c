
#include <assert.h>
#include <string.h>

#include "appengine/datastruct/array.h"

/* Periodically wipe regions within the specified block. */
static void array_memset_stride(unsigned char *base, int nelems, size_t width, int value)
{
  /* for certain values the alignment of base is predictable so that would
   * could conceivably move out of memset if it proves to be an overhead */

  /* e.g. if base is aligned to 4 and width is also 4 then we could just
   * rattle through memory stashing whole words */

  while (nelems--)
  {
    memset(base, value, width);
    base += width;
  }
}

/*
 * |A|  ->  |AB|  ->  |A |
 * |B|      |CD|      |B |
 * |C|      |  |      |C |
 * |D|      |  |      |D |
 */

/* walk backwards, skip last element */
void array_stretch1(unsigned char *base, int nelems, size_t oldwidth, size_t newwidth, int wipe_value)
{
  int i;

  assert(oldwidth < newwidth);

  for (i = nelems - 1; i > 0; i--)
    memmove(base + i * newwidth, base + i * oldwidth, oldwidth);

  array_memset_stride(base + oldwidth, nelems, newwidth - oldwidth,
                      wipe_value);
}

void array_stretch2(unsigned char *base, int nelems, size_t oldwidth, size_t newwidth, int wipe_value)
{
  unsigned char *p, *q;

  assert(newwidth > oldwidth);

  p = base + (nelems - 1) * oldwidth;
  q = base + (nelems - 1) * newwidth;

  while (p > base)
  {
    memmove(q, p, oldwidth);

    p -= oldwidth;
    q -= newwidth;
  }

  array_memset_stride(base + oldwidth, nelems, newwidth - oldwidth,
                      wipe_value);
}
