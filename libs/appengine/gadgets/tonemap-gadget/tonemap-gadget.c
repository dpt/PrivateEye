/* --------------------------------------------------------------------------
 *    Name: tonemap-gadget.c
 * Purpose: ToneMap gadget
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/colourtrans.h"
#include "oslib/wimp.h"

#include "appengine/geom/box.h"
#include "appengine/app/choices.h"
#include "appengine/base/oserror.h"
#include "appengine/base/messages.h"
#include "appengine/graphics/tonemap.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/icon.h"

#include "appengine/gadgets/tonemap-gadget.h"

#ifdef TONEMAP_BUFFER_TO_SPRITE

/* This didn't feel any faster in practice than the simpler version. */

/* Note that also because it buffers in a 1bpp sprite, it won't let us use
 * colour tones in the future. */

/* ----------------------------------------------------------------------- */

enum
{
  Border = 12  /* size of an icon border */
};

/* ----------------------------------------------------------------------- */

int tonemapgadget_update(wimp_w w, wimp_i i)
{
  wimp_window_state state;

  state.w = w;
  wimp_get_window_state(&state);

  if (state.flags & wimp_WINDOW_OPEN)
  {
    os_box b;

    icon_get_bbox((int) w, i, &b);
    box_grow(&b, -Border); /* compensate the bbox for size of border */
    wimp_force_redraw(w, b.x0, b.y0, b.x1, b.y1);
  }

  return 0;
}

static osspriteop_area *area = NULL;

static int makespr(tonemap *map)
{
  enum
  {
    w       = 128,
    h       = 128,
    log2bpp = 0,
  };

  int c0,c1,c2,c3;

  if (area == NULL)
  {
    size_t rowbytes;
    size_t imgbytes;

    rowbytes = (((w << log2bpp) + 31) & ~31) >> 3;
    imgbytes = sizeof(osspriteop_area) + sizeof(osspriteop_header);
    imgbytes += rowbytes * h;

    area = malloc(imgbytes);
    if (area == NULL)
    {
      return 1;
    }

    /* init area */

    area->size  = imgbytes;
    area->first = 16;
    osspriteop_clear_sprites(osspriteop_USER_AREA, area);

    /* create sprite */

    osspriteop_create_sprite(osspriteop_USER_AREA,
                             area,
                            "tonemap",
                             0, /* no palette */
                             w, h,
                             os_MODE1BPP90X90);
  }

  /* redirect to sprite */

  osspriteop_switch_output_to_sprite(osspriteop_USER_AREA,
                                     area,
                    (osspriteop_id) "tonemap",
                                     0,
                                     &c0, &c1, &c2, &c3);

  tonemap_draw(map, 0, 0);

  osspriteop_unswitch_output(c0, c1, c2, c3);

  return 0;
}

int tonemapgadget_redraw(tonemap *map, wimp_w w, wimp_i i, wimp_draw *redraw)
{
  static const os_palette palette[] = { os_COLOUR_BLACK, os_COLOUR_WHITE };

  os_box screenbox;
  int    more;

  os_factors factors = {1,1,1,1}; /* FIXME: Not mode independent. */

  byte trans_tab[256];

  colourtrans_generate_table(os_MODE1BPP90X90,
                             palette,
                             os_CURRENT_MODE,
                             colourtrans_CURRENT_PALETTE,
    (osspriteop_trans_tab *) trans_tab,
                             0,
                             NULL, NULL);

  icon_get_screen_bbox((int) w, i, &screenbox);
  box_grow(&screenbox, -Border); /* compensate the bbox for size of border */

  makespr(map);

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
  {
    if (!box_overlap(&screenbox, &redraw->clip))
      continue;

    osspriteop_put_sprite_scaled(osspriteop_PTR,
                                 area,
                 (osspriteop_id) sprite_select(area, 0),
                                 screenbox.x0, screenbox.y0,
                                 os_ACTION_OVERWRITE,
                                &factors,
        (osspriteop_trans_tab *) trans_tab);
  }

  return 0;
}

#else

/* ----------------------------------------------------------------------- */

enum
{
  Fudge  = 4,
  Border = 8  /* size of an icon border */
};

/* ----------------------------------------------------------------------- */

static int redraw(tonemap *map, wimp_w w, wimp_i i, wimp_draw *draw, osbool (*draw_fn)(wimp_draw *))
{
  os_box screenbox;
  int    more;

  icon_get_screen_bbox(w, i, &screenbox);
  box__grow(&screenbox, -Border); /* compensate the bbox for size of border */

  for (more = draw_fn(draw); more; more = wimp_get_rectangle(draw))
  {
    os_box clippedbox;

    if (!box__intersects(&screenbox, &draw->clip))
      continue;

    box__intersection(&draw->clip, &screenbox, &clippedbox);

    /* set our clip rectangle */
    screen_clip(&clippedbox);

    tonemap_draw(map, screenbox.x0 + Fudge, screenbox.y0 + Fudge);

    /* restore the previous clip rectangle */
    screen_clip(&draw->clip);
  }

  return 0;
}

int tonemapgadget_update(tonemap *map, wimp_w w, wimp_i i)
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
  box__grow(&u.update.box, -Border);

  u.update.clip = u.update.box;

  return redraw(map, w, i, &u.update, wimp_update_window);
}

int tonemapgadget_redraw(tonemap *map, wimp_w w, wimp_i i, wimp_draw *draw)
{
  return redraw(map, w, i, draw, wimp_redraw_window);
}

#endif
