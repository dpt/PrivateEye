/* --------------------------------------------------------------------------
 *    Name: scale.c
 * Purpose: Scale
 * Version: $Id: scale.c,v 1.26 2009-05-21 22:45:38 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/os.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/graphics/drawable.h"
#include "appengine/dialogues/scale.h"

#include "globals.h"
#include "viewer.h"

#include "scale.h"

int scale_for_box(drawable *d, int sw, int sh)
{
  static const os_factors one_to_one = { 1, 1, 1, 1 };

  os_box box;
  int    iw, ih;
  int    z;

  d->methods.get_dimensions(d, &one_to_one, &box);
  iw = box.x1 - box.x0;
  ih = box.y1 - box.y0;

  z = sw * 100 / iw;
  if (ih * z / 100 > sh)
    z = sh * 100 / ih;

  return z;
}

/* ----------------------------------------------------------------------- */

dialogue_t *scale;

/* ----------------------------------------------------------------------- */

void scale_set(viewer *viewer, int scale, int redraw)
{
  viewer_update_flags flags;

  if (scale == viewer->scale.cur)
    return;

  window_capture(viewer->main_w, &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  viewer->scale.prev = viewer->scale.cur;
  viewer->scale.cur  = scale;

  flags = viewer_UPDATE_SCALING | viewer_UPDATE_EXTENT;
  if (redraw)
    flags |= viewer_UPDATE_REDRAW;

  viewer_update(viewer, flags);

  viewer_open(viewer);
}

/* ----------------------------------------------------------------------- */

static void scale_fillout(dialogue_t *d, void *arg)
{
  viewer *viewer;
  image  *image;

  NOT_USED(arg);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  scale__set_bounds(d, image->scale.min, image->scale.max);

  scale__set(d, viewer->scale.cur);
}

static void scale_set_fit_screen(dialogue_t *d, viewer *viewer)
{
  int sw,sh;
  int s;

  read_max_visible_area(viewer->main_w, &sw, &sh);

  s = scale_for_box(viewer->drawable, sw, sh);

  scale__set(d, s);
}

static void scale_set_fit_window(dialogue_t *d, viewer *viewer)
{
  wimp_window_state state;
  int ww,wh;
  int s;

  state.w = viewer->main_w;
  wimp_get_window_state(&state);

  ww = state.visible.x1 - state.visible.x0;
  wh = state.visible.y1 - state.visible.y0;

  s = scale_for_box(viewer->drawable, ww, wh);

  scale__set(d, s);
}

static void scale_handler(dialogue_t *d, scale__type type, int scale)
{
  viewer *viewer;

  NOT_USED(d);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  if (scale == viewer->scale.cur)
    return;

  switch (type)
  {
  case scale__TYPE_VALUE:
    scale_set(viewer, scale, 1 /* redraw */);
    break;

  case scale__TYPE_FIT_TO_SCREEN:
    scale_set_fit_screen(d, viewer);
    break;

  case scale__TYPE_FIT_TO_WINDOW:
    scale_set_fit_window(d, viewer);
    break;
  }
}

/* ----------------------------------------------------------------------- */

error viewer_scale_init(void)
{
  scale = scale__create();
  if (scale == NULL)
    return error_OOM;

  dialogue__set_fillout_handler(scale, scale_fillout, NULL);
  scale__set_steppings(scale, GLOBALS.choices.scale.step,
                              GLOBALS.choices.scale.mult);
  scale__set_scale_handler(scale, scale_handler);

  return error_OK;
}

void viewer_scale_fin(void)
{
  scale__destroy(scale);
}
