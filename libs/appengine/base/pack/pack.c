/* --------------------------------------------------------------------------
 *    Name: pack.c
 * Purpose: Structure packing
 * Version: $Id: pack.c,v 1.3 2010-01-09 21:51:16 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "appengine/base/pack.h"

int pack(unsigned char* buf, const char* fmt, ...)
{
  va_list        args;
  unsigned char *bp;
  const char    *p;

  va_start(args, fmt);

  bp = buf;

  for (p = fmt; *p != '\0'; )
  {
    int c;
    int n;

    c = *p++;

    if (c == '*') /* array mode */
    {
      /* next arg is length of array */
      n = va_arg(args, int);
      c = *p++;

      switch (c)
      {
      case 'c': /* char */
        {
          const unsigned char *a; /* array */

          a = (const unsigned char *) va_arg(args, unsigned char *);

          memcpy(bp, a, n);
          bp += n;
        }
        break;

      case 's': /* short */
        {
          const unsigned short *a;

          a = (const unsigned short *) va_arg(args, unsigned short *);

          while (n--)
          {
            unsigned short v;

            v = *a++;
            *bp++ = (unsigned char) (v      ); /* little endian */
            *bp++ = (unsigned char) (v >>  8);
          }
        }
        break;

      case 'i': /* int */
        {
          const unsigned int *a;

          a = (const unsigned int *) va_arg(args, unsigned int *);

          while (n--)
          {
            unsigned int v;

            v = *a++;
            *bp++ = (unsigned char) (v      ); /* little endian */
            *bp++ = (unsigned char) (v >>  8);
            *bp++ = (unsigned char) (v >> 16);
            *bp++ = (unsigned char) (v >> 24);
          }
        }
        break;

      default:
        assert("pack: Illegal array format specifier" == NULL);
        va_end(args);
        return -1;
      }
    }
    else /* N single characters */
    {
      /* get repeat count, if any */
      if (isdigit(c))
      {
        n = c - '0';
        for (;;)
        {
          c = *p++;
          if (!isdigit(c)) break;
          n = n * 10 + (c - '0');
        }
      }
      else
      {
        /* n wasn't specified: assume the default of one */
        n = 1;
      }

      switch (c)
      {
      case 'c': /* char */
        while (n--)
          *bp++ = (unsigned char) va_arg(args, unsigned int);
        break;

      case 's': /* short */
        {
          unsigned int v;

          while (n--)
          {
            v = (short) va_arg(args, unsigned int);
            *bp++ = (unsigned char) (v); /* little endian */
            *bp++ = (unsigned char) (v >> 8);
          }
        }
        break;

      case 'i': /* int */
        {
          unsigned int v;

          while (n--)
          {
            v = va_arg(args, unsigned int);
            *bp++ = (unsigned char) (v); /* little endian */
            *bp++ = (unsigned char) (v >> 8);
            *bp++ = (unsigned char) (v >> 16);
            *bp++ = (unsigned char) (v >> 24);
          }
        }
        break;

      default: /* illegal type character */
        assert("pack: Illegal format specifier" == NULL);
        va_end(args);
        return -1;
      }
    }
  }

  va_end(args);

  return bp - buf;
}
