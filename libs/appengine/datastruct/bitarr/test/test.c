
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/datastruct/bitarr.h"

#define NBITS 97

typedef bitarr_ARRAY(NBITS) TestBits;

static void dumpbits(const TestBits *arr, size_t nbits)
{
  int i;
  int c;

  c = 0;
  for (i = nbits - 1; i >= 0; i--)
  {
    int b;

    b = bitarr__get(arr, i);
    c += b;
    printf("%d", b);
  }

  printf("\n%d bits set\n", c);
}

int bitarr_test(void)
{
  TestBits arr;
  int      i;

  printf("test: create\n");

  bitarr__wipe(arr, sizeof(arr));


  printf("test: set\n");

  for (i = 0; i < NBITS; i++)
    bitarr__set(&arr, i);

  dumpbits(&arr, NBITS);


  printf("test: clear\n");

  for (i = 0; i < NBITS; i++)
    bitarr__clear(&arr, i);

  dumpbits(&arr, NBITS);


  printf("test: toggle\n");

  for (i = 0; i < NBITS; i++)
    bitarr__toggle(&arr, i);

  dumpbits(&arr, NBITS);


  printf("test: count\n");

  /* a cast is required :-| */
  printf("%d bits set\n", bitarr__count((struct bitarr_t *) &arr, sizeof(arr)));


  return 0;
}
