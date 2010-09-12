/* --------------------------------------------------------------------------
 *    Name: unpack.c
 * Purpose: Structure unpacking
 * Version: $Id: unpack.c,v 1.3 2010-01-09 21:51:16 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "appengine/base/pack.h"

static int vunpack_le(const unsigned char* buf, const char* fmt, va_list args)
{
  const unsigned char *bp;
  const char          *p;

  bp = buf;

  for (p = fmt; *p != '\0'; )
  {
    int c;
    int n;

    c = *p++;

    if (c == '*') /* array mode */
    {
      /* next arg is number of elements to unpack to array */
      n = va_arg(args, int);
      c = *p++;

      switch (c)
      {
      case 'c': /* char */
        {
          unsigned char *a;

          a = (unsigned char *) va_arg(args, unsigned char *);

          memcpy(a, bp, n);
          bp += n;
        }
        break;

      case 's': /* short */
        {
          unsigned short *a;

          a = (unsigned short *) va_arg(args, unsigned short *);

          while (n--)
          {
            *a++ = (unsigned short) (bp[0] | (bp[1] << 8));
            bp += 2;
          }
        }
        break;

      case 'i': /* int */
        {
          unsigned int *a;

          a = (unsigned int *) va_arg(args, unsigned int *);

          while (n--)
          {
            *a++ = bp[0] | (bp[1] << 8) | (bp[2] << 16) | (bp[3] << 24);
            bp += 4;
          }
        }
        break;

      default:
        assert("unpack: Illegal format specifier" == NULL);
        return -1;
      }
    }
    else
    {
      if (isdigit(c))
      {
        /* get repeat count, if any */
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
        /* default repeat count */
        n = 1;
      }

      switch (c)
      {
      case 'c': /* char */
        while (n--)
        {
          unsigned char *pc;

          pc = va_arg(args, unsigned char *);
          *pc = *bp++;
        }
        break;

      case 's': /* short */
        while (n--)
        {
          unsigned short *ps;

          ps = va_arg(args, unsigned short *);
          *ps = bp[0] | (bp[1] << 8);
          bp += 2;
        }
        break;

      case 'i': /* int */
        while (n--)
        {
          unsigned int *pl;

          pl = va_arg(args, unsigned int *);
          *pl = bp[0] | (bp[1] << 8) | (bp[2] << 16) | (bp[3] << 24);
          bp += 4;
        }
        break;

      default:
        assert("unpack: Illegal format specifier" == NULL);
        return -1;
      }
    }
  }

  return bp - buf;
}

static int vunpack_be(const unsigned char* buf, const char* fmt, va_list args)
{
  const unsigned char *bp;
  const char          *p;

  bp = buf;

  for (p = fmt; *p != '\0'; )
  {
    int c;
    int n;

    c = *p++;

    if (c == '*') /* array mode */
    {
      /* next arg is number of elements to output to array */
      n = va_arg(args, int);
      c = *p++;

      switch (c)
      {
      case 'c': /* char */
        {
          unsigned char *a;

          a = (unsigned char *) va_arg(args, unsigned char *);

          memcpy(a, bp, n);
          bp += n;
        }
        break;

      case 's': /* short */
        {
          unsigned short *a;

          a = (unsigned short *) va_arg(args, unsigned short *);

          while (n--)
          {
            *a++ = (unsigned short) (bp[1] | (bp[0] << 8));
            bp += 2;
          }
        }
        break;

      case 'i': /* int */
        {
          unsigned int *a;

          a = (unsigned int *) va_arg(args, unsigned int *);

          while (n--)
          {
            *a++ = bp[3] | (bp[2] << 8) | (bp[1] << 16) | (bp[0] << 24);
            bp += 4;
          }
        }
        break;

      default:
        assert("unpack: Illegal format specifier" == NULL);
        return -1;
      }
    }
    else
    {
      if (isdigit(c))
      {
        /* get repeat count, if any */
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
        /* default repeat count */
        n = 1;
      }

      switch (c)
      {
      case 'c': /* char */
        while (n--)
        {
          unsigned char *pc;

          pc = va_arg(args, unsigned char *);
          *pc = *bp++;
        }
        break;

      case 's': /* short */
        while (n--)
        {
          unsigned short *ps;

          ps = va_arg(args, unsigned short *);
          *ps = bp[1] | (bp[0] << 8);
          bp += 2;
        }
        break;

      case 'i': /* int */
        while (n--)
        {
          unsigned int *pl;

          pl = va_arg(args, unsigned int *);
          *pl = bp[3] | (bp[2] << 8) | (bp[1] << 16) | (bp[0] << 24);
          bp += 4;
        }
        break;

      default:
        assert("unpack: Illegal format specifier" == NULL);
        return -1;
      }
    }
  }

  return bp - buf;
}

#ifdef ENDIAN_LITTLE
#define NATIVE_UNPACKER vunpack_le
#else
#define NATIVE_UNPACKER vunpack_be
#endif

int unpack(const unsigned char *buf, const char *fmt, ...)
{
  va_list args;
  int     c;
  int   (*unpacker)(const unsigned char *, const char *, va_list);

  unpacker = vunpack_le;

  if (*fmt == '=')
  {
    unpacker = NATIVE_UNPACKER;
    fmt++;
  }
  else if (*fmt == '<')
  {
    unpacker = vunpack_le;
    fmt++;
  }
  else if (*fmt == '>')
  {
    unpacker = vunpack_be;
    fmt++;
  }

  va_start(args, fmt);

  c = unpacker(buf, fmt, args);

  va_end(args);

  return c;
}

