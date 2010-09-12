/* --------------------------------------------------------------------------
 *    Name: name.c
 * Purpose: Turning colours to names
 * Version: $Id: name.c,v 1.1 2009-04-28 23:32:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "oslib/os.h"
#include "oslib/colourtrans.h"

#include "appengine/types.h"

#include "appengine/graphics/colour.h"

typedef struct
{
  os_colour   colour;
  const char *name;
}
element;

#define NCOLOURS 140

/* Names are taken from HTML */
static const element palette[NCOLOURS] =
{
  { 0x00000000, "black" },
  { 0x00008000, "maroon" },
  { 0x00008B00, "dark red" },
  { 0x0000FF00, "red" },
  { 0x0045FF00, "orange red" },
  { 0x00640000, "dark green" },
  { 0x00800000, "green" },
  { 0x00808000, "olive" },
  { 0x008CFF00, "dark orange" },
  { 0x00A5FF00, "orange" },
  { 0x00D7FF00, "gold" },
  { 0x00FC7C00, "lawn green" },
  { 0x00FF0000, "lime" },
  { 0x00FF7F00, "chartreuse" },
  { 0x00FFFF00, "yellow" },
  { 0x0B86B800, "dark goldenrod" },
  { 0x13458B00, "saddle brown" },
  { 0x1E69D200, "chocolate" },
  { 0x20A5DA00, "goldenrod" },
  { 0x2222B200, "fire brick" },
  { 0x228B2200, "forest green" },
  { 0x238E6B00, "olive drab" },
  { 0x2A2AA500, "brown" },
  { 0x2D52A000, "sienna" },
  { 0x2F6B5500, "dark olive green" },
  { 0x2FFFAD00, "green yellow" },
  { 0x32CD3200, "lime green" },
  { 0x32CD9A00, "yellow green" },
  { 0x3C14DC00, "crimson" },
  { 0x3F85CD00, "peru" },
  { 0x4763FF00, "tomato" },
  { 0x4F4F2F00, "dark slate grey" },
  { 0x507FFF00, "coral" },
  { 0x578B2E00, "sea green" },
  { 0x5C5CCD00, "indian red" },
  { 0x60A4F400, "sandy brown" },
  { 0x69696900, "dim grey" },
  { 0x6BB7BD00, "dark khaki" },
  { 0x70191900, "midnight blue" },
  { 0x71B33C00, "medium sea green" },
  { 0x7280FA00, "salmon" },
  { 0x7A96E900, "dark salmon" },
  { 0x7AA0FF00, "light salmon" },
  { 0x7FFF0000, "spring green" },
  { 0x80000000, "navy" },
  { 0x80008000, "purple" },
  { 0x80800000, "teal" },
  { 0x80808000, "grey" },
  { 0x8080F000, "light coral" },
  { 0x82004B00, "indigo" },
  { 0x8515C700, "medium violet red" },
  { 0x87B8DE00, "burlywood" },
  { 0x8B000000, "dark blue" },
  { 0x8B008B00, "dark magenta" },
  { 0x8B3D4800, "dark slate blue" },
  { 0x8B8B0000, "dark cyan" },
  { 0x8CB4D200, "tan" },
  { 0x8CE6F000, "khaki" },
  { 0x8F8FBC00, "rosybrown" },
  { 0x8FBC8D00, "dark sea green" },
  { 0x90EE9000, "light green" },
  { 0x9314FF00, "deep pink" },
  { 0x9370DB00, "pale violet red" },
  { 0x98FB9800, "pale green" },
  { 0x99887700, "light slate grey" },
  { 0x9AFA0000, "medium spring green" },
  { 0xA09E5F00, "cadet blue" },
  { 0xA9A9A900, "dark grey" },
  { 0xAAB22000, "light sea green" },
  { 0xAACD6600, "medium aquamarine" },
  { 0xAAE8EE00, "pale goldenrod" },
  { 0xADDEFF00, "navajo white" },
  { 0xB3DEF500, "wheat" },
  { 0xB469FF00, "hotpink" },
  { 0xB4824600, "steel blue" },
  { 0xB5E4FF00, "moccasin" },
  { 0xB9DAFF00, "peachpuff" },
  { 0xC0C0C000, "silver" },
  { 0xC1B6FF00, "light pink" },
  { 0xC4E4FF00, "bisque" },
  { 0xCBC8FF00, "pink" },
  { 0xCC329900, "dark orchid" },
  { 0xCCD14800, "medium turquoise" },
  { 0xCD000000, "medium blue" },
  { 0xCD5A6A00, "slate blue" },
  { 0xCDEBFF00, "blanched almond" },
  { 0xCDFAFF00, "lemon chiffon" },
  { 0xD0E04000, "turquoise" },
  { 0xD1DE0000, "dark turquoise" },
  { 0xD2FAFA00, "light goldenrod yellow" },
  { 0xD3009400, "dark violet" },
  { 0xD355BA00, "medium orchid" },
  { 0xD3D3D300, "light grey" },
  { 0xD4FF7F00, "aquamarine" },
  { 0xD5EFFF00, "papaya whip" },
  { 0xD670DA00, "orchid" },
  { 0xD7EBFA00, "antique white" },
  { 0xD8BFD800, "thistle" },
  { 0xDB709300, "medium purple" },
  { 0xDCDCDC00, "gainsboro" },
  { 0xDCF5F500, "beige" },
  { 0xDCF8FF00, "corn silk" },
  { 0xDDA0DD00, "plum" },
  { 0xDEC4B000, "light steel blue" },
  { 0xE0FFFF00, "light yellow" },
  { 0xE1694100, "royal blue" },
  { 0xE1E4FF00, "misty rose" },
  { 0xE22B8A00, "blue violet" },
  { 0xE6D8AD00, "light blue" },
  { 0xE6E0B000, "powder blue" },
  { 0xE6F0FA00, "linen" },
  { 0xE6F5FD00, "oldlace" },
  { 0xEBCE8700, "sky blue" },
  { 0xED956400, "cornflower blue" },
  { 0xEE687B00, "medium slate blue" },
  { 0xEE82EE00, "violet" },
  { 0xEEEEAF00, "pale turquoise" },
  { 0xEEF5FF00, "seashell" },
  { 0xF0FAFF00, "floral white" },
  { 0xF0FFF000, "honeydew" },
  { 0xF0FFFF00, "ivory" },
  { 0xF5F0FF00, "lavender blush" },
  { 0xF5F5F500, "white smoke" },
  { 0xFACE8700, "light sky blue" },
  { 0xFAE6E600, "lavender" },
  { 0xFAFAFF00, "snow" },
  { 0xFAFFF500, "mint cream" },
  { 0xFF000000, "blue" },
  { 0xFF00FF00, "fuchsia" },
  { 0xFF00FF00, "magenta" },
  { 0xFF901E00, "dodger blue" },
  { 0xFFBF0000, "deep sky blue" },
  { 0xFFF8F000, "alice blue" },
  { 0xFFF8F800, "ghost white" },
  { 0xFFFF0000, "aqua" },
  { 0xFFFF0000, "cyan" },
  { 0xFFFFE000, "light cyan" },
  { 0xFFFFF000, "azure" },
  { 0xFFFFFF00, "white" },
};

enum
{
  weight_RED   = 19595, /* 0.29900 * 65536 (rounded down) */
  weight_GREEN = 38470, /* 0.58700 * 65536 (rounded up)   */
  weight_BLUE  =  7471, /* 0.11400 * 65536 (rounded down) */
};

const char *colour_to_name(os_colour c)
{
  static os_colour    previous_c     = 0xFFFFFF00;
  static unsigned int previous_besti = NCOLOURS - 1;

  int          cr, cg, cb;
  int          i;
  unsigned int best;
  int          besti;

  if (previous_c == c)
    return palette[previous_besti].name;

  /* 0xBBGGRR00 */

  cr = (c >> 8)  & 0xFF;
  cg = (c >> 16) & 0xFF;
  cb = (c >> 24) & 0xFF;

  best = INT_MAX;
  besti = -1;

  for (i = 0; i < NCOLOURS; i++)
  {
    os_colour    p;
    int          pr, pg, pb;
    unsigned int d;

    p = palette[i].colour;

    pr = (p >> 8)  & 0xFF;
    pg = (p >> 16) & 0xFF;
    pb = (p >> 24) & 0xFF;

    /* compute distance / error */

    d = ((cr - pr) * (cr - pr)) * weight_RED   +
        ((cg - pg) * (cg - pg)) * weight_GREEN +
        ((cb - pb) * (cb - pb)) * weight_BLUE;

    if (d >= best)
      continue; /* already have something at least as good */

    if (d == 0)
      return palette[i].name;

    best  = d;
    besti = i;
  }

  previous_c     = c;
  previous_besti = besti;

  return palette[besti].name;
}

typedef struct
{
  int          index;
  unsigned int score;
}
rec;

static int compare(const void *a, const void *b)
{
  const rec *ca = a, *cb = b;

  if (ca->score < cb->score)
    return -1;
  else if (ca->score > cb->score)
    return +1;
  else
    return 0;
}

char *colour_to_approx(os_colour c)
{
  static os_colour previous = 0xFFFFFFFF;

  rec colours[NCOLOURS]; /* 140 * 2 * 4 = 1120 */
  int cr, cg, cb;
  int i;

  if (previous == c)
  {
    // return previous result;
  }

  cr = (c >> 8)  & 0xFF;
  cg = (c >> 16) & 0xFF;
  cb = (c >> 24) & 0xFF;

  for (i = 0; i < NCOLOURS; i++)
  {
    os_colour    p;
    int          pr, pg, pb;
    unsigned int d;

    p = palette[i].colour;

    pr = (p >> 8)  & 0xFF;
    pg = (p >> 16) & 0xFF;
    pb = (p >> 24) & 0xFF;

    /* compute distance / error */

    d = ((cr - pr) * (cr - pr)) * weight_RED   +
        ((cg - pg) * (cg - pg)) * weight_GREEN +
        ((cb - pb) * (cb - pb)) * weight_BLUE;

    colours[i].index = i;
    colours[i].score = d;
  }

  qsort(colours, NCOLOURS, sizeof(colours[0]), compare);

  if (0)
  {
    FILE *f;

    f = fopen("colour-dump", "w");
    for (i = 0; i < NCOLOURS; i++)
    {
      fprintf(f,"%d: %d %x\n", i, colours[i].index, colours[i].score);

      colourtrans_set_gcol(palette[colours[i].index].colour,
                           colourtrans_USE_ECFS_GCOL,
                           os_ACTION_OVERWRITE,
                           NULL);

      os_plot(os_MOVE_TO, i * 16, 0);
      os_plot(os_PLOT_RECTANGLE | os_PLOT_BY, 16 - 1, 16 - 1);
    }
    fclose(f);
  }

  if (colours[0].score == 0) /* perfect match */
    return (char*) palette[colours[0].index].name;

  return "WTF";
}
