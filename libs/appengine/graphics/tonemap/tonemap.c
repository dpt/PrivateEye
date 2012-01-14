/* --------------------------------------------------------------------------
 *    Name: tonemap.c
 * Purpose: Tone maps
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "fortify/fortify.h"

#include "oslib/draw.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"

#include "appengine/graphics/tonemap.h"

enum
{
  Segments      = 32,
  PathSize      = ((2 + (Segments - 1) + 1 + 1) * 3 + 1) * 4,
  MaxComponents = 4, /* red, green, blue, alpha */
};

/* ----------------------------------------------------------------------- */

typedef unsigned int tonemap_internal_flags;

#define FLAG_RGB_IN_SYNC       (1u << 0) /* RGB components in sync */
#define FLAG_TABLES_UP_TO_DATE (1u << 2)
#define FLAG_PATHS_UP_TO_DATE  (1u << 3)

typedef struct tonemap_comp
{
  tonemap_spec        spec;
  os_correction_table table;
}
tonemap_comp;

struct tonemap
{
  tonemap_internal_flags flags;
  int                    ncomponents;
  tonemap_comp           comp[MaxComponents];
  int                   *path;
  size_t                 pathsz; /* bytes */
  int                    width;  /* Draw units */
};

/* ----------------------------------------------------------------------- */

tonemap *tonemap_create(void)
{
  tonemap *map;

  map = malloc(sizeof(*map));
  if (map == NULL)
    return NULL;

  map->path   = NULL;
  map->pathsz = 0;
  map->width  = 8;

  /* tonemap_reset initialises the structure for us */

  tonemap_reset(map);

  return map;
}

void tonemap_destroy(tonemap *map)
{
  if (map == NULL)
    return;

  free(map->path);
  free(map);
}

void tonemap_reset(tonemap *map)
{
  tonemap_comp *comp;

  map->flags         = FLAG_RGB_IN_SYNC; /* (re-)build tables, paths */
  map->ncomponents   = 3;

  for (comp = map->comp; comp < map->comp + MaxComponents; comp++)
  {
    comp->spec.flags      = 0;   /* invert, reflect */
    comp->spec.gamma      = 100;
    comp->spec.brightness = 100;
    comp->spec.midd       = 50;
    comp->spec.contrast   = 100;
    comp->spec.bias       = 50;
    comp->spec.gain       = 50;
  }

  /* leave any path in place to avoid having to reallocate it later on */
}

tonemap *tonemap_copy(const tonemap *map)
{
  tonemap *new_map;

  new_map = tonemap_create();
  if (new_map == NULL)
    return NULL;

  /* copy the tonemap, but not the path data.
   * set the flags such that it will be (re-)created when next requested */

  new_map->flags       = map->flags & ~FLAG_PATHS_UP_TO_DATE;
  new_map->ncomponents = map->ncomponents;
  memcpy(new_map->comp, map->comp, sizeof(map->comp));
  new_map->width       = map->width;

  return new_map;
}

/* ----------------------------------------------------------------------- */

static void calc(tonemap *map)
{
  tonemap_comp *comp;

  if (map->flags & FLAG_TABLES_UP_TO_DATE)
    return;

  for (comp = map->comp; comp < map->comp + map->ncomponents; comp++)
  {
      double  igamma; /* inverse */
      int     brightness;
      int     contrast;
      int     midd;
      double  dbias, dgain;
      byte   *tab;
      int     i;

      /* inverse of gamma, hence gamma correction */
      igamma     = 100.0 / comp->spec.gamma;

      brightness = comp->spec.brightness - 50;
      brightness = (int) (brightness * 2.56);

      contrast   = (int) (comp->spec.contrast * 2.56);

      midd  = comp->spec.midd;

      dbias = comp->spec.bias / 100.0;
      dbias = CLAMP(dbias, 0.00001, 0.99999);

      dgain = comp->spec.gain / 100.0;
      dgain = CLAMP(dgain, 0.00001, 0.99999);

      tab = comp->table.gamma;

      for (i = 0; i < 256; i++)
      {
        int j;

        j = (int) ceil(pow(i / 255.0, igamma) * 255.0);

        j = contrast * (j - 128) / 256;

        j += brightness;

        if (i < 128)
          j = j * midd / 50;
        else
          j = (midd * 255 / 100) + (j - 128) * (100 - midd) / 50;

        //j = CLAMP(j, 0, 255);

        //j = CLAMP(j, 0, 255);

        if (comp->spec.bias != 50 || comp->spec.gain != 50)
        {
          double x, t, b, g;

          /* gamma/constrast/brightness result is input to bias */

          x = j / 255.0;
          t = (1.0 / dbias - 2) * (1.0 - x);
          b = x / (t + 1);

          /* bias result is input to gain */

          x = b;
          t = (1.0 / dgain - 2) * (1.0 - 2.0 * x);
          if (x < 0.5)
            g = x / (t + 1.0);
          else
            g = (t - x) / (t - 1.0);

          j = (int) (g * 255.0);

          //j = CLAMP(j, 0, 255);
        }

        j = CLAMP(j, 0, 255);
        tab[i] = j;
      }

      if (comp->spec.flags & tonemap_FLAG_INVERT)
      {
        for (i = 0; i < 127; i++)
        {
          int t;

          t = tab[i];
          tab[i] = tab[255 - i];
          tab[255 - i] = t;
        }
      }

      if (comp->spec.flags & tonemap_FLAG_REFLECT)
      {
        for (i = 0; i < 127; i++)
          tab[255 - i] = tab[i];
      }

      if (comp == map->comp && map->flags & FLAG_RGB_IN_SYNC)
        comp = map->comp + 2; /* skip forward to alpha stage (note comp++) */
  }

  if (map->flags & FLAG_RGB_IN_SYNC)
  {
    os_correction_table *src;
    size_t               sz;

    /* we know that R, G and B are in sync so we skipped over the redundant
     * table generation stage. now copy the table computed for R into G and
     * B. */

    src = &map->comp[0].table;
    sz  = sizeof(map->comp[0].table);

    memcpy(&map->comp[1].table, src, sz);
    memcpy(&map->comp[2].table, src, sz);
  }

  map->flags |= FLAG_TABLES_UP_TO_DATE;
}

void tonemap_set(tonemap         *map,
                 tonemap_channels channels,
           const tonemap_spec    *spec)
{
  tonemap_internal_flags intflags;
  tonemap_internal_flags rgbflags;
  int                    ncomponents;
  tonemap_comp          *comp;
  int                    i;

  intflags = map->flags;

  /* if R + G + B are set simultaneously then remember that */

  rgbflags = channels & tonemap_CHANNEL_RGB;
  if (rgbflags == tonemap_CHANNEL_RGB)
    intflags |= FLAG_RGB_IN_SYNC;
  else if (rgbflags != 0)
    intflags &= ~FLAG_RGB_IN_SYNC;
  /* else r/g/b not being modified */

  /* see if we need to store alpha */

  ncomponents = (channels & tonemap_CHANNEL_ALPHA) ? 4 : 3;
  if (ncomponents > map->ncomponents)
    map->ncomponents = ncomponents;

  /* update all of the requested components */

  for (comp = map->comp, i = 0; i < map->ncomponents; i++, comp++)
  {
    if ((channels & (1u << i)) == 0)
      continue;

    comp->spec = *spec;
  }

  /* tables and paths are now potentially stale */

  intflags &= ~(FLAG_TABLES_UP_TO_DATE | FLAG_PATHS_UP_TO_DATE);

  map->flags = intflags;
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  int         index;  /* index into map->comp[] */
  int         stroked;
  wimp_colour colour;
}
comp_path_style;

static error prepare_paths(tonemap               *map,
                           int                    npaths,
                           const comp_path_style *indices)
{
  int    c;
  size_t need;

#define MOVETO(pd,px,py) do { *pd++ = draw_MOVE_TO; \
                              *pd++ = (px) * 256;   \
                              *pd++ = (py) * 256; } while (0)

#define LINETO(pd,px,py) do { *pd++ = draw_LINE_TO; \
                              *pd++ = (px) * 256;   \
                              *pd++ = (py) * 256; } while (0)

  /* ensure that the tables are up to date */

  calc(map);

  /* avoid where possible */

  if (map->flags & FLAG_PATHS_UP_TO_DATE)
    return error_OK;

  /* allocate */

  need = PathSize * npaths;

  if (map->path == NULL || map->pathsz < need)
  {
    free(map->path);
    map->pathsz = 0;

    map->path = malloc(need);
    if (map->path == NULL)
      return error_OOM;

    map->pathsz = need;
  }

  /* generate */

  for (c = 0; c < npaths; c++)
  {
    byte *tab;
    int   step;
    int  *p;
    int   i;

    tab = map->comp[indices[c].index].table.gamma;

    step = 256 / Segments; /* should be constant (avoid division) */

    p = (int *) ((char *) map->path + PathSize * c);

    if (indices[c].stroked)
    {
      MOVETO(p, 0, tab[0]);
    }
    else
    {
      MOVETO(p, 0, 0);
      LINETO(p, 0, tab[0]);
    }

    for (i = step; i < 256; i += step)
      LINETO(p, i, tab[i]);

    LINETO(p, 255, tab[255]);

    if (!indices[c].stroked)
      LINETO(p, 255, 0);

    *p++ = draw_END_PATH;
  }

  map->flags |= FLAG_PATHS_UP_TO_DATE;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static const comp_path_style style_r_g_b[] = /* R + G + B */
{
  { 0, 1, wimp_COLOUR_RED        },
  { 1, 1, wimp_COLOUR_DARK_GREEN },
  { 2, 1, wimp_COLOUR_DARK_BLUE  },
};

static const comp_path_style style_all[] = /* RGB */
{
  { 0, 0, wimp_COLOUR_BLACK      },
};

static const comp_path_style style_r_g_b_a[] = /* R + G + B + A */
{
  { 0, 1, wimp_COLOUR_RED        },
  { 1, 1, wimp_COLOUR_DARK_GREEN },
  { 2, 1, wimp_COLOUR_DARK_BLUE  },
  { 3, 1, wimp_COLOUR_DARK_GREY  },
};

static const comp_path_style style_all_a[] = /* RGB + A */
{
  { 0, 0, wimp_COLOUR_BLACK      },
  { 3, 1, wimp_COLOUR_DARK_GREY  },
};

typedef struct
{
  const comp_path_style *indices;
  int                    npaths;
}
style_map_ent;

static const style_map_ent style_map[2][2] =
{
  {
    { style_r_g_b,   NELEMS(style_r_g_b)   },
    { style_all,     NELEMS(style_all)     },
  },
  {
    { style_r_g_b_a, NELEMS(style_r_g_b_a) },
    { style_all_a,   NELEMS(style_all_a)   },
  },
};

static const style_map_ent *get_style_map(const tonemap *map)
{
  int alpha;
  int sync;

  alpha = map->ncomponents >= 4;
  sync  = (map->flags & FLAG_RGB_IN_SYNC) != 0;

  return &style_map[alpha][sync];
}

error tonemap_draw(tonemap *map, int x, int y)
{
  error                  err;
  int                    npaths;
  const comp_path_style *indices;
  const style_map_ent   *ent;
  os_trfm                t;
  int                    c;

  ent = get_style_map(map);

  npaths  = ent->npaths;
  indices = ent->indices;

  err = prepare_paths(map, npaths, indices);
  if (err)
    return err;

  t.entries[0][0] = 1 << 16;
  t.entries[0][1] = 0;
  t.entries[1][0] = 0;
  t.entries[1][1] = 1 << 16;
  t.entries[2][0] = x * draw_OS_UNIT;
  t.entries[2][1] = y * draw_OS_UNIT;

  /* clear the background to white */

  wimp_set_colour(wimp_COLOUR_WHITE | (1 << 7));
  os_writec(os_VDU_CLG);

  /* draw the gamma curves */

  for (c = 0; c < npaths; c++)
  {
    wimp_set_colour(indices[c].colour);

    if (!indices[c].stroked)
    {
      draw_fill((draw_path *) map->path, draw_FILL_NONZERO, &t, 0);
    }
    else
    {
      static const draw_line_style line_style =
      {
        draw_JOIN_BEVELLED, draw_CAP_BUTT, draw_CAP_BUTT, 0, 0, 0, 0, 0, 0,
      };

      draw_path *path;

      path = (draw_path *) ((char *) map->path + PathSize * c);

      draw_stroke(path, draw_FILL_NONZERO, &t, 0, map->width << 8, &line_style, NULL);
    }
  }

  return error_OK;
}

void tonemap_draw_set_stroke_width(tonemap *map, int width)
{
  map->width = width;
}

/* ----------------------------------------------------------------------- */

void tonemap_get_corrections(tonemap                    *map,
                             const os_correction_table **red,
                             const os_correction_table **green,
                             const os_correction_table **blue,
                             const os_correction_table **alpha)
{
  static const os_correction_table linear =
  {
    {
      0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
      0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
      0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
      0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
      0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
      0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
      0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
      0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
      0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
      0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
      0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
      0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
      0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
      0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
      0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
      0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
      0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
      0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
      0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
      0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
      0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
      0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
      0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
      0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
      0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
      0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
      0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
      0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
      0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
      0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
      0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
      0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
    }
  };

  calc(map);

  if (red)   *red   = &map->comp[0].table;
  if (green) *green = &map->comp[1].table;
  if (blue)  *blue  = &map->comp[2].table;
  if (alpha)
  {
    if (map->ncomponents >= 4)
      *alpha = &map->comp[3].table;
    else
      *alpha = &linear;
  }
}

/* ----------------------------------------------------------------------- */

void tonemap_get_values(tonemap         *map,
                        tonemap_channels channels,
                        tonemap_spec    *spec)
{
  int i;

  switch (channels)
  {
  case tonemap_CHANNEL_RGB:
  case tonemap_CHANNEL_RED:   i = 0; break;
  case tonemap_CHANNEL_GREEN: i = 1; break;
  case tonemap_CHANNEL_BLUE:  i = 2; break;
  case tonemap_CHANNEL_ALPHA: i = 3; break;
  default:
    assert(0);
    return;
  }

  *spec = map->comp[i].spec;
}
