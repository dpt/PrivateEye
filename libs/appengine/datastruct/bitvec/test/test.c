/* $Id: test.c,v 1.2 2008-08-08 22:41:05 dpt Exp $ */

#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"
#include "appengine/datastruct/bitvec.h"

#define NBITS 97

static void dumpbits(bitvec_t *v)
{
  int i;

  for (i = bitvec__length(v) - 1; i >= 0; i--)
    printf("%d", bitvec__get(v, i));
  printf("\n");
}

int bitvec_test(void)
{
  error     err;
  bitvec_t *v;
  int       i;
  bitvec_t *w;
  bitvec_t *x;

  printf("test: create\n");

  v = bitvec__create(0);
  if (!v)
    return 1;

  printf("test: set\n");

  for (i = 0; i < NBITS; i++)
  {
    err = bitvec__set(v, i);
    if (err)
      return 1;
  }

  printf("test: get\n");

  {
    int c;

    c = 0;
    for (i = 0; i < NBITS; i++)
      c += bitvec__get(v, i);

    printf("%d bits set\n", c);
  }

  printf("test: count\n");

  printf("%d bits set\n", bitvec__count(v));

  printf("test: clear\n");

  for (i = 0; i < NBITS; i++)
    bitvec__clear(v, i);

  printf("test: get\n");

  {
    int c;

    c = 0;
    for (i = 0; i < NBITS; i++)
      c += bitvec__get(v, i);

    printf("%d bits set\n", c);
  }

  printf("test: count\n");

  printf("%d bits set\n", bitvec__count(v));

  printf("test: length\n");

  printf("length=%d bits\n", bitvec__length(v));

  printf("test: next\n");

  printf("set every 11th bit\n");

  for (i = 0; i < NBITS; i += 11)
  {
    err = bitvec__set(v, i);
    if (err)
      return 1;
  }

  printf("walk through set bits\n");

  i = -1;
  do
  {
    printf("after %d ", i);
    i = bitvec__next(v, i);
    printf("comes %d\n", i);
  }
  while (i != -1);

  printf("test: destroy\n");

  bitvec__destroy(v);

  printf("test: eq\n");

  v = bitvec__create(0);
  if (!v)
    return 1;

  w = bitvec__create(0);
  if (!w)
    return 1;

  printf("created. equal? %d\n", bitvec__eq(v, w));

  err = bitvec__set(v, 1);
  err = bitvec__set(w, 1);

  printf("set bit 1 for v+w. equal? %d\n", bitvec__eq(v, w));

  err = bitvec__set(v, 1000);
  err = bitvec__set(w, 1000);

  printf("set bit 1000 for v+w. equal? %d\n", bitvec__eq(v, w));

  bitvec__clear(v, 1000);

  printf("clear bit 1000 for v. equal? %d\n", bitvec__eq(v, w));

  bitvec__clear(w, 1000);

  printf("clear bit 1000 for w. equal? %d\n", bitvec__eq(v, w));

  /* this tests that bitvecs with different internal lengths still compare
   * equal */

  bitvec__destroy(w);

  w = bitvec__create(0);
  if (!w)
    return 1;

  bitvec__clear(v, 1);

  printf("clear bit 1 for v. recreate w. equal? %d\n", bitvec__eq(v, w));

  bitvec__destroy(w);

  bitvec__destroy(v);

  printf("test: and\n");

  v = bitvec__create(0);
  if (!v)
    return 1;

  w = bitvec__create(0);
  if (!w)
    return 1;

  err = bitvec__set(v, 32);
  if (err)
    return 1;

  err = bitvec__set(w, 32);
  if (err)
    return 1;

  err = bitvec__and(v, w, &x);
  if (err)
    return 1;

  dumpbits(x);

  bitvec__destroy(x);
  bitvec__destroy(w);
  bitvec__destroy(v);

  printf("test: or\n");

  v = bitvec__create(0);
  if (!v)
    return 1;

  w = bitvec__create(0);
  if (!w)
    return 1;

  err = bitvec__set(v, 0);
  if (err)
    return 1;

  err = bitvec__set(w, 32);
  if (err)
    return 1;

  err = bitvec__or(v, w, &x);
  if (err)
    return 1;

  dumpbits(x);

  bitvec__destroy(x);
  bitvec__destroy(w);
  bitvec__destroy(v);

  return 0;
}
