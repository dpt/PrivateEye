/* --------------------------------------------------------------------------
 *    Name: tonemap-gadget.c
 * Purpose: ToneMap gadget
 * ----------------------------------------------------------------------- */

#include <string.h>

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/draw.h"
#include "oslib/wimp.h"

#include "framebuf/tonemap.h"
#include "geom/box.h"

#include "appengine/types.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/icon.h"

#include "appengine/gadgets/tonemap-gadget.h"

/* ----------------------------------------------------------------------- */

enum
{
  Fudge    = 4,
  Border   = 8, /* size of an icon border */
  Segments = 32,
  PathLen  = (2 + (Segments - 1) + 1 + 1) * 3 + 1, /* words */
};

/* ----------------------------------------------------------------------- */

typedef struct
{
  int         index;   /* index into the tables */
  int         stroked;
  wimp_colour colour;
}
comp_path_style;

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

/* ----------------------------------------------------------------------- */

static void draw_curve(const tonemap_table_t *table,
                       const comp_path_style *style,
                       const os_trfm         *t,
                       int                    stroke_width)
{
  static const draw_line_style line_style =
  {
    draw_JOIN_BEVELLED, draw_CAP_BUTT, draw_CAP_BUTT, 0, 0, 0, 0, 0, 0,
  };

  int  path[PathLen];
  int *p;
  int  i;

#define MOVETO(pd,px,py) do { *pd++ = draw_MOVE_TO; \
                              *pd++ = (px) * 256;   \
                              *pd++ = (py) * 256; } while (0)

#define LINETO(pd,px,py) do { *pd++ = draw_LINE_TO; \
                              *pd++ = (px) * 256;   \
                              *pd++ = (py) * 256; } while (0)

  p = path;

  if (style->stroked)
  {
    MOVETO(p, 0, table->level[0]);
  }
  else
  {
    MOVETO(p, 0, 0);
    LINETO(p, 0, table->level[0]);
  }

  for (i = 256 / Segments; i < 256; i += 256 / Segments)
    LINETO(p, i, table->level[i]);

  LINETO(p, 255, table->level[255]);

  if (!style->stroked)
    LINETO(p, 255, 0);

  *p++ = draw_END_PATH;

#undef MOVETO
#undef LINETO

  wimp_set_colour(style->colour);

  if (!style->stroked)
    draw_fill((draw_path *) path, draw_FILL_NONZERO, t, 0);
  else
    draw_stroke((draw_path *) path, draw_FILL_NONZERO, t, 0,
                stroke_width << 8, &line_style, NULL);
}

static void draw_tonemap(tonemap_t         *map,
                         tonemap_channels_t channels,
                         int                x,
                         int                y,
                         int                stroke_width)
{
  const tonemap_table_t *tables[4];
  int                    sync;
  int                    i;
  int                    alpha;
  const style_map_ent   *ent;
  os_trfm                t;

  tonemap_get_tables(map, &tables[0], &tables[1], &tables[2], &tables[3]);

  /* With no RGB channel selected (editing alpha) fall back to comparing the
   * tables. */
  if ((channels & tonemap_CHANNEL_RGB) == 0)
    sync = memcmp(tables[0], tables[1], sizeof(*tables[0])) == 0 &&
           memcmp(tables[1], tables[2], sizeof(*tables[0])) == 0;
  else
    sync = (channels & tonemap_CHANNEL_RGB) == tonemap_CHANNEL_RGB;

  /* Draw alpha when it's selected or isn't linear. */
  for (i = 0; i < 256 && tables[3]->level[i] == i; i++)
    ;
  alpha = (channels & tonemap_CHANNEL_ALPHA) != 0 || i < 256;

  ent = &style_map[alpha][sync];

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

  for (i = 0; i < ent->npaths; i++)
    draw_curve(tables[ent->indices[i].index], &ent->indices[i], &t,
               stroke_width);
}

/* ----------------------------------------------------------------------- */

static int redraw(tonemap_t         *map,
                  tonemap_channels_t channels,
                  int                stroke_width,
                  wimp_w             w,
                  wimp_i             i,
                  wimp_draw         *draw,
                  osbool           (*draw_fn)(wimp_draw *))
{
  os_box screenbox;
  int    more;

  icon_get_screen_bbox(w, i, &screenbox);
  box_grow(&screenbox, -Border); /* compensate the bbox for size of border */

  for (more = draw_fn(draw); more; more = wimp_get_rectangle(draw))
  {
    os_box clippedbox;

    if (!box_intersects(&screenbox, &draw->clip))
      continue;

    box_intersection(&draw->clip, &screenbox, &clippedbox);

    /* set our clip rectangle */
    screen_clip(&clippedbox);

    draw_tonemap(map, channels, screenbox.x0 + Fudge, screenbox.y0 + Fudge,
                 stroke_width);

    /* restore the previous clip rectangle */
    screen_clip(&draw->clip);
  }

  return 0;
}

int tonemapgadget_update(tonemap_t         *map,
                         tonemap_channels_t channels,
                         int                stroke_width,
                         wimp_w             w,
                         wimp_i             i)
{
  union
  {
    wimp_window_state state;
    wimp_draw         update;
  }
  u;

  u.state.w = w;
  wimp_get_window_state(&u.state);

  if ((u.state.flags & wimp_WINDOW_OPEN) == 0)
    return 0;

  icon_get_bbox(w, i, &u.update.box);

  /* compensate the bbox for size of border */
  box_grow(&u.update.box, -Border);

  u.update.clip = u.update.box;

  return redraw(map, channels, stroke_width, w, i, &u.update,
                wimp_update_window);
}

int tonemapgadget_redraw(tonemap_t         *map,
                         tonemap_channels_t channels,
                         int                stroke_width,
                         wimp_w             w,
                         wimp_i             i,
                         wimp_draw         *draw)
{
  return redraw(map, channels, stroke_width, w, i, draw, wimp_redraw_window);
}

