/* $Id: test.c,v 1.1 2010-01-09 21:51:16 dpt Exp $ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/strings.h"

#include "appengine/base/pack.h"

int pack_test(void)
{
  int nfailures = 0;

  printf("test: pack and unpack\n");

  {
    static const int  data[] = { -0x80, 0x7f,
                                 -0x8000, 0x7fff,
                                 INT_MIN, INT_MAX };

    static const char fmt[] = "2c2s2i";
    const int         sz = 14;

    unsigned char     buf[100];
    int               n;
    signed char       c1, c2;
    short             s1, s2;
    int               l1, l2;

    n = pack(buf, fmt, data[0], data[1], data[2], data[3], data[4], data[5]);
    if (n != sz)
    {
      printf("Failure: pack() returned unexpected size\n");
      nfailures++;
    }

    n = unpack(buf, fmt, &c1, &c2, &s1, &s2, &l1, &l2);
    if (n != sz || c1 != data[0] || c2 != data[1] ||
             s1 != data[2] || s2 != data[3] ||
             l1 != data[4] || l2 != data[5])
    {
      printf("Failure: unpack():\n"
            "%d %d, %d %d, %d %d, %d %d, %d %d, %d %d\n",
            c1, data[0], c2, data[1],
            s1, data[2], s2, data[3],
            l1, data[4], l2, data[5]);
      nfailures++;
    }
  }

  printf("test: unpack (array mode)\n");

  {
    static const short         shorts[]       = { -0x8000, 0x7fff,
                                                   0x0000, 0x5555 };
    static const unsigned char packedshorts[] = {  0x80, 0x00, 0x7f, 0xff,
                                                   0x00, 0x00, 0x55, 0x55 };

    short                      unpackedshorts[4];
    int                        n;

    /* unpacking a big-endian array of shorts */

    n = unpack(packedshorts, ">*s", NELEMS(unpackedshorts), unpackedshorts);
    if (n != 8 || unpackedshorts[0] != shorts[0] ||
            unpackedshorts[1] != shorts[1] ||
            unpackedshorts[2] != shorts[2] ||
            unpackedshorts[3] != shorts[3])
    {
      printf("Failure: unpack() array:\n"
            "%d %d, %d %d, %d %d, %d %d\n",
            unpackedshorts[0], shorts[0],
            unpackedshorts[1], shorts[1],
            unpackedshorts[2], shorts[2],
            unpackedshorts[3], shorts[3]);
      nfailures++;
    }
  }

  printf("test: pack (array mode)\n");

  {
    static const unsigned char  cdata[2]     = { 0x00, 0xff };
    static const unsigned short sdata[2]     = { 0x5500, 0xf0ff };
    static const unsigned int   ldata[2]     = { 0x00005500, 0xff00ffff };
    static const unsigned char  expected[14] = { 0x00, 0xff,
                                                 0x00, 0x55,
                                                 0xff, 0xf0,
                                                 0x00, 0x55,
                                                 0x00, 0x00,
                                                 0xff, 0xff,
                                                 0x00, 0xff };
    static const char           fmt[]        = "*c*s*i";
    const int                   sz           = 2 + 4 + 8;

    unsigned char               buf[100];
    int                         n;

    n = pack(buf, fmt, 2, cdata, 2, sdata, 2, ldata);
    if (n != sz)
    {
      printf("Failure: pack() using '*' returned unexpected size (%d)\n", n);
      nfailures++;
    }

    if (memcmp(buf, expected, sz) != 0)
    {
      printf("Failure: pack() using '*'\n");
      nfailures++;
    }
  }

  return nfailures > 0;
}
