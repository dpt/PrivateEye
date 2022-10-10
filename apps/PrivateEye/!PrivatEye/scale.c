/* --------------------------------------------------------------------------
 *    Name: scale.c
 * Purpose: Viewer scale dialogue handler
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/os.h"

#include "appengine/dialogues/scale.h"
#include "appengine/graphics/drawable.h"
#include "appengine/wimp/dialogue.h"

#include "globals.h"
#include "viewer.h"

#include "scale.h"

int drawable_best_fit_for_size(drawable_t *d, int w, int h)
{
  static const os_factors one_to_one = { 1, 1, 1, 1 };

  os_box box;
  int    dw, dh; /* drawable width, height */
  int    rp; /* ratio percent */

  d->methods.get_dimensions(d, &one_to_one, &box);
  dw = box.x1 - box.x0;
  dh = box.y1 - box.y0;

  rp = w * 100 / dw;
  if (dh * rp / 100 > h)
    rp = h * 100 / dh;

  return rp;
}

/* ----------------------------------------------------------------------- */

dialogue_t *viewer_scaledlg;

/* ----------------------------------------------------------------------- */

void viewer_scaledlg_set(viewer_t *viewer, int scale, int redraw)
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

static void viewer_scaledlg_fillout(dialogue_t *d, void *opaque)
{
  viewer_t *viewer;
  image_t  *image;

  NOT_USED(opaque);

  viewer = GLOBALS.current_viewer;
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  scale_set_range(d, image->scale.min, image->scale.max);

  scale_set(d, viewer->scale.cur);
}

int viewer_scaledlg_fit_to_screen(viewer_t *viewer)
{
  int sw,sh;

  read_max_visible_area(viewer->main_w, &sw, &sh);
  if (!GLOBALS.choices.viewer.cover_icon_bar)
    sh -= read_icon_bar_unobscured();

  if (GLOBALS.choices.viewer.size == viewersize_FIT_TO_SCREEN)
  {
    int fixed_x, fixed_y;

    stage_get_fixed(&viewer->background.stage.config, &fixed_x, &fixed_y);
    sw -= fixed_x;
    sh -= fixed_y;
  }
  
  return drawable_best_fit_for_size(viewer->drawable, sw, sh);
}

static void viewer_scaledlg_set_fit_screen(dialogue_t *d, viewer_t *viewer)
{
  scale_set(d, viewer_scaledlg_fit_to_screen(viewer));
}

static void viewer_scaledlg_set_fit_window(dialogue_t *d, viewer_t *viewer)
{
  wimp_window_state state;
  int               ww,wh;
  int               s;

  state.w = viewer->main_w;
  wimp_get_window_state(&state);

  ww = state.visible.x1 - state.visible.x0;
  wh = state.visible.y1 - state.visible.y0;

  s = drawable_best_fit_for_size(viewer->drawable, ww, wh);

  scale_set(d, s);
}

static void viewer_scaledlg_handler(dialogue_t *d,
                                    scale_type  type,
                                    int         scale)
{
  viewer_t *viewer;

  NOT_USED(d);

  viewer = GLOBALS.current_viewer;
  if (viewer == NULL)
    return;

  if (scale == viewer->scale.cur)
    return;

  switch (type)
  {
  case scale_TYPE_VALUE:
    viewer_scaledlg_set(viewer, scale, 1 /* redraw */);
    break;

  case scale_TYPE_FIT_TO_SCREEN:
    viewer_scaledlg_set_fit_screen(d, viewer);
    break;

  case scale_TYPE_FIT_TO_WINDOW:
    viewer_scaledlg_set_fit_window(d, viewer);
    break;
  }
}

/* ----------------------------------------------------------------------- */

result_t viewer_scaledlg_init(void)
{
  dialogue_t *scale;

  scale = scale_create();
  if (scale == NULL)
    return result_OOM;

  dialogue_set_fillout_handler(scale, viewer_scaledlg_fillout, NULL);
  scale_set_steppings(scale,
                      GLOBALS.choices.scale.step,
                      GLOBALS.choices.scale.mult);
  scale_set_scale_handler(scale, viewer_scaledlg_handler);

  viewer_scaledlg = scale;

  return result_OK;
}

void viewer_scaledlg_fin(void)
{
  scale_destroy(viewer_scaledlg);
}
