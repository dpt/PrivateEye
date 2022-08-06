/* --------------------------------------------------------------------------
 *    Name: jpeg-utils.c
 * Purpose: JPEG loader
 * ----------------------------------------------------------------------- */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "oslib/osbyte.h"
#include "oslib/osmodule.h"

#include "jpeg.h"

/* ----------------------------------------------------------------------- */

/* Returns the specified module's version number as BCD, or -1 if not found.
 */
static int get_module_version(const char *wanted)
{
  os_error *e;
  int       module_no;
  int       section;
  char     *module_name;
  int       bcd_version;

  module_no = 0;
  section   = -1;
  for (;;)
  {
    e = xosmodule_enumerate_rom_with_info(module_no,
                                          section,
                                         &module_no,
                                         &section,
                                         &module_name,
                                          NULL, /* status */
                                          NULL, /* chunk_no */
                                         &bcd_version);
    if (e)
      return -1; /* unknown */

    if (strcmp(module_name, wanted) == 0)
      return bcd_version;
  }
}

/* Returns non-zero when the OS has progressive JPEG support. */
static int progressive_jpeg_supported(void)
{
  int os     = osbyte1(osbyte_IN_KEY, 0, 0xFF);
  int sprext = get_module_version("SpriteExtend");

  switch (os)
  {
    /* RISC OS Select 2 with SpriteExtend 1.30 or greater */
    case 0xA9: return (sprext >= 0x00013000);
    /* RISC OS 5 with SpriteExtend 1.73 or greater */
    case 0xAA: return (sprext >= 0x00017300);
    default: return 0;
  }
}

/* ----------------------------------------------------------------------- */

int jpeg_get_info(const unsigned char *jpeg_data,
                  int                  file_size,
                  jpeg_info_t         *info)
{
  const unsigned char *p;
  const unsigned char *q;
  int                  size;
  const unsigned char *r;
  ptrdiff_t            l;
  jpeg_flags_t         flags = jpeg_FLAG_TRUNCATED;
  int                  ncomponents = -1;
  char                 componentids[4];
  int                  app14_colortransform = -1;
  jpeg_colourspace_t   colourspace;
  int                  i;

  enum
  {
    R_SOF0  = (1 <<  0),
    R_SOF1  = (1 <<  1),
    R_SOF2  = (1 <<  2),
    R_SOF9  = (1 <<  9), /* Extended Sequential + Arithmetic coding */
    R_SOF10 = (1 << 10), /* Progressive + Arithmetic coding */
    R_DHT   = (1 << 16),
    R_SOI   = (1 << 17),
    R_EOI   = (1 << 18),
    R_SOS   = (1 << 19),
    R_DQT   = (1 << 20),
    R_APP0  = (1 << 21),
    R_APP1  = (1 << 22),
    R_APP2  = (1 << 23),
    R_APP14 = (1 << 23)
  };

  p = jpeg_data;
  q = p + file_size;

  for (; p < q; p += size)
  {
    if (p[0] != 0xFF) /* we're expecting a marker */
      break;

    switch (p[1])
    {
    case M_SOF0:  /* Baseline */
    case M_SOF1:  /* Extended sequential */
    case M_SOF2:  /* Progressive */
    case M_SOF9:  /* Extended sequential + Arithmetic coding */
    case M_SOF10: /* Progressive + Arithmetic coding */
      switch (p[1])
      {
        case M_SOF0:  flags |= jpeg_FLAG_BASELINE;                   break;
        case M_SOF1:  flags |= jpeg_FLAG_EXTSEQ;                     break;
        case M_SOF2:  flags |= jpeg_FLAG_PRGRSSVE;                   break;
        case M_SOF9:  flags |= jpeg_FLAG_EXTSEQ   | jpeg_FLAG_ARITH; break;
        case M_SOF10: flags |= jpeg_FLAG_PRGRSSVE | jpeg_FLAG_ARITH; break;
      }
      ncomponents = p[9];
      for (i = 0; i < ncomponents; i++)
        componentids[i] = p[10 + i*3];
      break;

    case M_APP0:
      size = (p[2] << 8) + p[3] + 2;
      if (size >= (2 + 2 + 4 + 1 + 2 + 1 + 2 + 2 + 1 + 1) && memcmp(p + 4, "JFIF\0", 5) == 0)
        flags |= jpeg_FLAG_JFIF;
      break;

    case M_APP1:
      size = (p[2] << 8) + p[3] + 2;
      if (size > 10 && memcmp(p + 4, "Exif\0\0", 6) == 0)
        flags |= jpeg_FLAG_EXIF;
      break;

    case M_APP14:
      size = (p[2] << 8) + p[3] + 2;
      if (size == (2 + 2 + 5 + 2 + 2 + 2 + 1) && memcmp(p + 4, "Adobe", 5) == 0)
        flags |= jpeg_FLAG_ADOBE;
      app14_colortransform = p[15];
      break;
    }

    switch (p[1])
    {
    case M_SOI: /* SOI may occur more than once */
      size = 2;
      break;
    case M_EOI:
      flags &= ~jpeg_FLAG_TRUNCATED;
      goto end;

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
        goto end; /* truncated */

      r = memchr(p, 0xFF, l);
      if (r == NULL)
        goto end; /* end of segment not found */

      size = r - p;
      break;

    default:
      size = (p[2] << 8) + p[3] + 2;
      break;
    }
  }

end:
  switch (ncomponents)
  {
  case 1:
    colourspace = jpeg_COLOURSPACE_GREYSCALE;
    break;

  case 3:
    {
      static const jpeg_colourspace_t map[4] = 
      {
        jpeg_COLOURSPACE_RGB,    /* Adobe says "Unknown (RGB or CMYK)" */
        jpeg_COLOURSPACE_YCBCR,  /* Adobe says YCbCr */
        jpeg_COLOURSPACE_UNKNOWN /* Adobe says YCCK */
      };

      switch (app14_colortransform)
      {
      case -1: /* No APP14 was seen so examine components */
        if (componentids[0] == 1 && componentids[1] == 2 && componentids[2] == 3)
          colourspace = jpeg_COLOURSPACE_YCBCR;
        else if (componentids[0] == 'R' && componentids[1] == 'G' && componentids[2] == 'B')
          colourspace = jpeg_COLOURSPACE_RGB;
        else
          colourspace = jpeg_COLOURSPACE_UNKNOWN;
        break;

      default:
        colourspace = map[app14_colortransform];
        break;
      }
    }
    break;

  case 4:
    {
      static const jpeg_colourspace_t map[4] = 
      {
        jpeg_COLOURSPACE_YCCK,    /* No APP14 was seen so assume YCCK */
        jpeg_COLOURSPACE_CMYK,    /* Adobe says "Unknown (RGB or CMYK)" */
        jpeg_COLOURSPACE_UNKNOWN, /* Adobe says YCbCr */
        jpeg_COLOURSPACE_YCCK     /* Adobe says YCCK */
      };
      colourspace = map[app14_colortransform + 1]; /* -1..2 -> 0..3 */
    }
    break;

  default:
    colourspace = jpeg_COLOURSPACE_UNKNOWN;
    break;
  }

  info->flags       = flags;
  info->colourspace = colourspace;

  return 0;
}

/**
 * Returns non-zero when the given JPEG can be handled by the OS.
 *
 * Progressive JPEGs are permitted if an appropriately capable version of
 * SpriteExtend is available. This requires RISC OS Select 2 and
 * SpriteExtend 1.30 or later, or RISC OS 5 and SpriteExtend 1.73 or later.
 *
 * Otherwise we need to detect progressive JPEGs.
 *
 * Sample JPEG tables:
 *   Baseline:
 *     SOI APP0 DQT DQT SOF0 DHT DHT DHT DHT SOS EOI
 *   Baseline optimised:
 *     SOI APP0 DQT DQT SOF0 DHT DHT DHT DHT SOS EOI
 *   Default progressive:
 *     SOI APP0 DQT DQT SOF2 DHT DHT SOS DHT SOS DHT SOS DHT SOS DHT SOS DHT
 *     SOS SOS DHT SOS DHT SOS DHT SOS EOI
 *   Progressive 2 scan:
 *     SOI APP0 DQT DQT SOF0 DHT DHT SOS DHT DHT SOS EOI
 *
 * See that SOF0 not SOF2 can appear in a 2-scan progressive image. It looks
 * like the best way to detect a progressive image is to check for the
 * presence of more than one SOS marker (makes sense...)
 */
int jpeg_supported(const jpeg_info_t *info)
{
  static int progressive_supported = -1;

  if (progressive_supported < 0)
    progressive_supported = progressive_jpeg_supported();

  if (info->flags & jpeg_FLAG_ARITH)
    return 0; /* not supported anywhere? TODO: CHECK */

  if (info->flags & jpeg_FLAG_PRGRSSVE)
    return progressive_supported;

  return 1; /* supported */
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
