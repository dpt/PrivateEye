/* --------------------------------------------------------------------------
 *    Name: scale.c
 * Purpose: Viewer scale dialogue
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

int viewer_scale_for_box(drawable *d, int sw, int sh)
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

dialogue_t *viewer_scale;

/* ----------------------------------------------------------------------- */

void viewer_scale_set(viewer_t *viewer, int scale, int redraw)
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

static void viewer_scale_fillout(dialogue_t *d, void *arg)
{
  viewer_t *viewer;
  image_t  *image;

  NOT_USED(arg);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  scale__set_bounds(d, image->scale.min, image->scale.max);

  scale__set(d, viewer->scale.cur);
}

static void viewer_scale_set_fit_screen(dialogue_t *d, viewer_t *viewer)
{
  int sw,sh;
  int s;

  read_max_visible_area(viewer->main_w, &sw, &sh);

  s = viewer_scale_for_box(viewer->drawable, sw, sh);

  scale__set(d, s);
}

static void viewer_scale_set_fit_window(dialogue_t *d, viewer_t *viewer)
{
  wimp_window_state state;
  int ww,wh;
  int s;

  state.w = viewer->main_w;
  wimp_get_window_state(&state);

  ww = state.visible.x1 - state.visible.x0;
  wh = state.visible.y1 - state.visible.y0;

  s = viewer_scale_for_box(viewer->drawable, ww, wh);

  scale__set(d, s);
}

static void viewer_scale_handler(dialogue_t *d, scale__type type, int scale)
{
  viewer_t *viewer;

  NOT_USED(d);

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  if (scale == viewer->scale.cur)
    return;

  switch (type)
  {
  case scale__TYPE_VALUE:
    viewer_scale_set(viewer, scale, 1 /* redraw */);
    break;

  case scale__TYPE_FIT_TO_SCREEN:
    viewer_scale_set_fit_screen(d, viewer);
    break;

  case scale__TYPE_FIT_TO_WINDOW:
    viewer_scale_set_fit_window(d, viewer);
    break;
  }
}

/* ----------------------------------------------------------------------- */

error viewer_scale_init(void)
{
  dialogue_t *scale;

  scale = scale__create();
  if (scale == NULL)
    return error_OOM;

  dialogue__set_fillout_handler(scale, viewer_scale_fillout, NULL);
  scale__set_steppings(scale, GLOBALS.choices.scale.step,
                              GLOBALS.choices.scale.mult);
  scale__set_scale_handler(scale, viewer_scale_handler);

  viewer_scale = scale;

  return error_OK;
}

void viewer_scale_fin(void)
{
  scale__destroy(viewer_scale);
}
