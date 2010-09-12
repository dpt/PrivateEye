/* $Id: test.c,v 1.1 2010-01-10 00:48:18 dpt Exp $ */

#include <stdio.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/text/txtfmt.h"

static const char *data[] =
{
  "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Donec mattis luctus libero. Donec imperdiet, velit quis venenatis iaculis, metus libero cursus ligula, egestas sagittis dui diam in mi.",
  "The\nQuick\nBrown\nFox",
};

static const int widths[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

static const int ndata = NELEMS(data);

int txtfmt_test(void)
{
  error     err;
  txtfmt_t *tx[ndata];
  int       i;

  printf("test: create\n");

  for (i = 0; i < ndata; i++)
  {
    err = txtfmt__create(data[i], &tx[i]);
    if (err)
      return 1;
  }

  printf("test: wrap, print\n");

  for (i = 0; i < ndata; i++)
  {
    int j;

    for (j = 0; j < NELEMS(widths); j++)
    {
      int w = widths[j];

      printf("test string %d, wrapping to %d:\n", i, w);

      err = txtfmt__wrap(tx[i], w);
      if (err)
        return 1;

      printf("height=%d\n", txtfmt__get_height(tx[i]));

      err = txtfmt__print(tx[i]);
      if (err)
        return 1;
    }
  }

  printf("test: destroy\n");

  for (i = 0; i < ndata; i++)
  {
    txtfmt__destroy(tx[i]);
  }

  return 0;
}
