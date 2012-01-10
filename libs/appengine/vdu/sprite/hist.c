/* --------------------------------------------------------------------------
 *    Name: hist.c
 * Purpose: Sprite histograms
 * ----------------------------------------------------------------------- */

#include <string.h>

#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

static void histogram_8(osspriteop_header *header,
                        int                width,
                        int                height,
                        sprite_histograms *hists)
{
  /* must be monochrome (pixels assumed to represent luma) */

  const int      histsz = sizeof(hists->h[0]);
  unsigned char *pixels;
  int            rowbytes;
  int            rowskip;
  int            x,y;

  memset(&hists->h[0], 0, histsz); /* zero the counters */

  pixels = sprite_data(header);

  /* account for wastage, need to align each scanline */

  rowbytes = (width + 3) & ~3;
  rowskip  = rowbytes - width;

  for (y = height; y--; )
  {
    for (x = width; x--; )
    {
      unsigned int p;

      p = *pixels++;

      hists->h[0].v[p]++;
    }

    pixels += rowskip;
  }

  memcpy(&hists->h[1], &hists->h[0], histsz); /* R */
  memcpy(&hists->h[2], &hists->h[0], histsz); /* G */
  memcpy(&hists->h[3], &hists->h[0], histsz); /* B */
  memset(&hists->h[4], 0, histsz);            /* A */
}

static void histogram_32(osspriteop_header *header,
                         int                width,
                         int                height,
                         sprite_histograms *hists)
{
  unsigned int *pixels;
  int           xy;

  memset(&hists->h[0], 0, sizeof(*hists)); /* zero the counters */

  pixels = sprite_data(header);

  /* no wastage in 32bpp sprites, so no need to align each scanline */

  for (xy = width * height; xy--; )
  {
    unsigned int p;
    unsigned int r,g,b,a;
    unsigned int i;

    p = *pixels++;

    r = (p      ) & 0xFF;
    g = (p >> 8 ) & 0xFF;
    b = (p >> 16) & 0xFF;
    a = (p >> 24) & 0xFF;

    hists->h[1].v[r]++;
    hists->h[2].v[g]++;
    hists->h[3].v[b]++;
    hists->h[4].v[a]++;

    /* Y = 0.29900 * R + 0.58700 * G + 0.11400 * B */
    /* Y =   19595 * R +   38470 * G +    7471 * B */

    i = (19595 * r + 38470 * g + 7471 * b) >> 16;

    hists->h[0].v[i]++;
  }
}

error sprite_get_histograms(osspriteop_area   *area,
                            osspriteop_header *header,
                            sprite_histograms *hists)
{
  int                   width, height;
  osspriteop_mode_word  mode;
  int                   log2bpp;
  void                (*fn)(osspriteop_header *, int, int, sprite_histograms *);

  sprite_info(area, header,
             &width, &height, NULL, (os_mode *) &mode, &log2bpp);

  switch (log2bpp)
  {
  case 3:
    fn = histogram_8;
    break;

  case 5:
    fn = histogram_32;
    break;

  default:
    return error_SPRITEFX_UNSUPP_FUNC;
  }

  fn(header, width, height, hists);

  return error_OK;
}
