/* --------------------------------------------------------------------------
 *    Name: jpeg-utils.c
 * Purpose: JPEG loader
 * Version: $Id: jpeg-utils.c,v 1.1 2009-04-28 23:32:24 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "jpeg.h"

/**
 * Verifies that the given JPEG can be handled by SpriteExtend
 * (ie. it isn't progressive).
 *
 * Sample JPEG tables.
 *
 * Baseline
 *   SOI APP0 DQT DQT SOF0 DHT DHT DHT DHT SOS EOI
 * Baseline optimised
 *   SOI APP0 DQT DQT SOF0 DHT DHT DHT DHT SOS EOI
 * Default progressive
 *   SOI APP0 DQT DQT SOF2 DHT DHT SOS DHT SOS DHT SOS DHT SOS DHT SOS DHT
 *   SOS SOS DHT SOS DHT SOS DHT SOS EOI
 * Progressive 2 scan
 *   SOI APP0 DQT DQT SOF0 DHT DHT SOS DHT DHT SOS EOI
 *
 * See that SOF0 not SOF2 can appear in a 2-scan progressive image. It looks
 * like the best way to detect a progressive image is to check for the
 * presence of more than one SOS marker.
 */
int jpeg_verify(const unsigned char *jpeg_data,
                int                  file_size,
                unsigned int        *mask)
{
  const unsigned char *p;
  const unsigned char *q;
  unsigned int         present;
  int                  size;
  unsigned int         essential;
  const unsigned char *r;
  ptrdiff_t            l;

  enum
  {
    R_SOF0 = 1 << 0,
    R_DHT  = 1 << 1,
    R_SOI  = 1 << 2,
    R_EOI  = 1 << 3,
    R_SOS  = 1 << 4,
    R_DQT  = 1 << 5,
    R_APP0 = 1 << 6,
    R_APP1 = 1 << 7
  };

  p = jpeg_data;
  q = p + file_size;

  present = 0;

  for (; p < q; p += size)
  {
    if (p[0] != 0xFF) /* we're expecting a marker */
      break;

#ifndef NDEBUG
    fprintf(stderr, "jpeg: %x\n", p[1]);
#endif

    switch (p[1])
    {
    case M_SOF0: present |= R_SOF0; break;
    case M_DHT:  present |= R_DHT;  break;
    case M_SOI:  present |= R_SOI;  break;
    case M_EOI:  present |= R_EOI;  break;
    case M_SOS:
      if (present & R_SOS)
        return 0; /* we've already seen an SOS: it's progressive */

      present |= R_SOS;
      break;
    case M_DQT:  present |= R_DQT;  break;
    case M_APP0: present |= R_APP0; break;
    case M_APP1: present |= R_APP1; break;
    }

    switch (p[1])
    {
    case M_SOI: /* SOI may occur more than once */
    case M_EOI: /* ### should i terminate at EOI? */
      size = 2;
      break;

    case 0:
    case M_SOS:
    case M_RST0:
    case M_RST1:
    case M_RST2:
    case M_RST3:
    case M_RST4:
    case M_RST5:
    case M_RST6:
    case M_RST7:
      /* these markers don't give a size. we'll need to scan forward for
       * the next marker */

      p += 2;
      l = q - p;
      if (l < 2) /* need at least two more bytes */
        goto whoops; /* truncated */

      r = memchr(p, 0xFF, l);
      if (r == NULL)
        goto whoops; /* end of segment not found */

      size = r - p;
      break;

    default:
      size = (p[2] << 8) + p[3] + 2;
      break;
    }
  }

whoops:

  *mask = present;

  essential = R_DHT + R_DQT + R_SOF0 + R_SOI + R_SOS + R_EOI;
  return (present & essential) == essential;
}

void jpeg_find(const unsigned char  *jpeg_data,
               int                   file_size,
               JPEG_MARKER           marker,
               const unsigned char **location,
               int                  *length,
               int                  *offset)
{
  const unsigned char *p;
  const unsigned char *q;
  int                  size;
  const unsigned char *r;
  ptrdiff_t            l;

  p = jpeg_data + *offset;
  q = p + file_size;

  for (; p < q; p += size)
  {
    if (p[0] != 0xFF) /* we're expecting a marker */
      break;

#ifndef NDEBUG
    fprintf(stderr, "jpeg_find: %x\n", p[1]);
#endif

    switch (p[1])
    {
    case M_SOI: /* SOI may occur more than once */
    case M_EOI:
      size = 2;
      break;

    case 0:
    case M_SOS:
    case M_RST0:
    case M_RST1:
    case M_RST2:
    case M_RST3:
    case M_RST4:
    case M_RST5:
    case M_RST6:
    case M_RST7:
      /* these markers don't give a size. we'll need to scan forward for
       * the next marker */

      p += 2;
      l = q - p;
      if (l < 2) /* need at least two more bytes */
        goto NotFound; /* truncated */

      r = memchr(p, 0xFF, l);
      if (r == NULL)
        goto NotFound; /* end of segment not found */

      size = r - p;
      break;

    default:
      /* the size given in the header includes the size itself, but not the
       * marker, so it's always two bytes shorter than the complete segment
       * length. */

      size = (p[2] << 8) + p[3] + 2;

      if (p[1] == marker)
      {
        *location = p + 4;
        *length   = size - 4;
        *offset   = p + size - jpeg_data;

        return;
      }

      break;
    }

  }

NotFound:
  *location = NULL;
  *length   = 0;
}
