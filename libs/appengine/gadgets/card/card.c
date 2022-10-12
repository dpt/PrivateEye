/* --------------------------------------------------------------------------
 *    Name: card.c
 * Purpose: Draws regions bordered by sprites
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "flex.h"

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/os.h"

#include "geom/box.h"

#include "appengine/types.h"
#include "appengine/gadgets/card.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"
#include "appengine/wimp/window.h"

/* ----------------------------------------------------------------------- */

typedef struct edge
{
  const char *name;
  int         x,y;
  enum
  {
    Single,
    TileH,
    TileV
  }           type;
}
edge;

#define NEDGES 16

static const struct edge edges[NEDGES] =
{
  { "tv-bl",  0,0, Single },
  { "tv-tl",  0,2, Single },
  { "tv-br",  2,0, Single },
  { "tv-tr",  2,2, Single },

  { "tv-b",   1,0, TileH  },
  { "tv-t",   1,2, TileH  },

  { "tv-l",   0,1, TileV  },
  { "tv-r",   2,1, TileV  },

  { "tv-bli", 0,0, Single },
  { "tv-tli", 0,2, Single },
  { "tv-bri", 2,0, Single },
  { "tv-tri", 2,2, Single },

  { "tv-bi",  1,0, TileH  },
  { "tv-ti",  1,2, TileH  },

  { "tv-li",  0,1, TileV  },
  { "tv-ri",  2,1, TileV  },
};

/* ----------------------------------------------------------------------- */

static struct
{
  int                   xpos[4];
  int                   ypos[4];
  osspriteop_header    *header[NEDGES];
  osspriteop_area      *area;
  osspriteop_trans_tab *trans_tab;
  os_factors            factors;
  int                   w,h;
}
LOCALS;

/* ----------------------------------------------------------------------- */

void card_prepare(int item_w, int item_h)
{
  osspriteop_area *area;
  int              i;
  int              w,h;
  os_mode          mode;

  area = window_get_sprite_area();

  LOCALS.area = area;

  if (LOCALS.trans_tab)
    flex_free((flex_ptr) &LOCALS.trans_tab);

  /* set up header ptrs */

  for (i = 0; i < NEDGES; i++)
  {
    LOCALS.header[i] = osspriteop_select_sprite(osspriteop_NAME,
                                                area,
                                (osspriteop_id) edges[i].name);
  }

  sprite_info(area, LOCALS.header[0], &w, &h, NULL, &mode, NULL);

  /* taking &area here is ok, as it's only used to deref in sprite_colours */
  sprite_colours(&area, LOCALS.header[0], &LOCALS.trans_tab); // NoMem

  {
    int         image_xeig, image_yeig;
    int         screen_xeig, screen_yeig;
    os_factors *scaled_factors;

    read_mode_vars(mode, &image_xeig, &image_yeig, NULL);
    read_current_mode_vars(&screen_xeig, &screen_yeig, NULL);

    scaled_factors = &LOCALS.factors;
    scaled_factors->xmul = image_xeig;
    scaled_factors->ymul = image_yeig;
    scaled_factors->xdiv = screen_xeig;
    scaled_factors->ydiv = screen_yeig;

    w <<= image_xeig;
    h <<= image_yeig;
  }

  LOCALS.w = w;
  LOCALS.h = h;

  LOCALS.xpos[0] = 0;
  LOCALS.xpos[1] = w;
  LOCALS.xpos[2] = item_w - w;
  LOCALS.xpos[3] = item_w;

  LOCALS.ypos[0] = 0;
  LOCALS.ypos[1] = h;
  LOCALS.ypos[2] = item_h - h;
  LOCALS.ypos[3] = item_h;
}

static void plot_edge(wimp_draw *redraw, int index, int x, int y)
{
  int    ex, ey;
  int    x0, y0;
  int    x1, y1;
  os_box clip;

  ex = edges[index].x;
  ey = edges[index].y;

  x0 = x + LOCALS.xpos[ex];
  y0 = y + LOCALS.ypos[ey];
  x1 = x + LOCALS.xpos[ex + 1];
  y1 = y + LOCALS.ypos[ey + 1];

  clip.x0 = x0;
  clip.y0 = y0;
  clip.x1 = x1;
  clip.y1 = y1;

  box_intersection(&clip, &redraw->clip, &clip);

  if (box_is_empty(&clip))
    return;

  (void) screen_clip(&clip);

  {
    int  lo, hi;
    int  step;
    int *xy;
    int  reps;
    int  i;

    if (edges[index].type == TileH)
    {
      lo   = x0;
      hi   = x1;
      step = LOCALS.w;
      xy   = &x0;
    }
    else /* TileV or Single */
    {
      lo   = y0;
      hi   = y1;
      step = LOCALS.h;
      xy   = &y0;
    }

    reps = ((hi + step - 1) - lo) / step;

    for (i = 0; i < reps; i++)
    {
      osspriteop_put_sprite_scaled(osspriteop_PTR,
                                   LOCALS.area,
                   (osspriteop_id) LOCALS.header[index],
                                   x0, y0,
                                   os_ACTION_OVERWRITE |
                                   os_ACTION_USE_MASK  |
                                   osspriteop_GIVEN_WIDE_ENTRIES,
                                  &LOCALS.factors,
                                   LOCALS.trans_tab);
      *xy += step;
    }
  }

  (void) screen_clip(&redraw->clip);
}

void card_draw(wimp_draw *redraw, int x, int y, card_draw_flags flags)
{
  int i;
  int o;

  if (flags & card_draw_flag_INVERT)
    o = 8;
  else
    o = 0;

  for (i = 0; i < 8; i++)
    plot_edge(redraw, o + i, x, y);

  colourtrans_set_gcol(os_COLOUR_WHITE, colourtrans_USE_ECFS_GCOL,
                       os_ACTION_OVERWRITE, NULL);

  os_plot(os_MOVE_TO,                     x + LOCALS.xpos[1], y + LOCALS.ypos[1]);
  os_plot(os_PLOT_TO | os_PLOT_RECTANGLE, x + LOCALS.xpos[2], y + LOCALS.ypos[2]);
}
