/* --------------------------------------------------------------------------
 *    Name: viewer.c
 * Purpose: Viewer block handling
 * ----------------------------------------------------------------------- */

#include "swis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/drawfile.h"
#include "oslib/hourglass.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"

#include "flex.h"

#include "geom/box.h"

#include "appengine/types.h"
#include "appengine/app/choices.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"
#include "appengine/datastruct/array.h"
#include "appengine/dialogues/dcs-quit.h"
#include "appengine/graphics/drawable.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/graphics/imagecache.h"
#include "appengine/graphics/stage.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/window.h"

#include "display.h"
#include "globals.h"
#include "privateeye.h"
#include "save.h"
#include "scale.h"
#include "viewer.h"
#include "zones.h"

/* Viewers are stored in a linked list. */
static list_t list_anchor = { NULL };

/* ----------------------------------------------------------------------- */

static void update_viewer_title(viewer_t *viewer)
{
  char file_name[256];
  char percentage[12];
  char buf[256];
  int  nclones;

  if (viewer->image->file_name[0] != '\0')
    sprintf(file_name, "%.*s",
      (int) sizeof(file_name),
            viewer->image->file_name);
  else
    strcpy(file_name, message0("untitled.title"));
  sprintf(percentage, "%d%%", viewer->scale.cur);
  sprintf(buf, message0("display.title"), file_name, percentage);

  if (viewer->image->flags & image_FLAG_MODIFIED)
    strcat(buf, " *");

  nclones = viewer_count_clones(viewer);
  if (nclones > 1)
  {
    char tmp[12];

    sprintf(tmp, " %d", nclones);
    strcat(buf, tmp);
  }

  window_set_title_text(viewer->main_w, buf);
}

/* ----------------------------------------------------------------------- */

static int refresh_all_titles_callback(viewer_t *viewer, void *opaque)
{
  NOT_USED(opaque);

  update_viewer_title(viewer);

  return 0;
}

static void refresh_all_titles(void)
{
  viewer_map(refresh_all_titles_callback, NULL);
}

/* ----------------------------------------------------------------------- */

result_t viewer_create(viewer_t **new_viewer)
{
  result_t       err;
  viewer_t      *v;
  wimp_w         w = wimp_ICON_BAR;
  stageconfig_t *sc;

  *new_viewer = NULL;

  /* Claim a viewer block */
  v = malloc(sizeof(*v));
  if (v == NULL)
    goto NoMem;

  /* Clone ourselves a window */
  w = window_clone(GLOBALS.display_w);
  if (w == NULL)
    goto NoMem;

  v->main_w          = w;
#ifdef EYE_ZONES
  v->zones           = NULL;
#endif
  v->drawable        = NULL;
  v->image           = NULL;
  v->capture.flags   = 0;
  v->scrolling.count = 0;

  sc = &v->background.stage.config; 
  stageconfig_init(sc);
  sc->pasteboard_min = GLOBALS.choices.viewer.stage.pasteboard.minsize;
  sc->stroke         = GLOBALS.choices.viewer.stage.stroke.size;
  sc->margin         = GLOBALS.choices.viewer.stage.margin.size;
  sc->shadow         = GLOBALS.choices.viewer.stage.shadow.size;

  if (GLOBALS.choices.viewer.scale == viewerscale_PRESERVE)
    /* 'Preserve' means take the last scale.
     * We have no previous scale set so use 100%. */
    v->scale.cur = v->scale.prev = 100;
  else
    v->scale.cur = v->scale.prev = GLOBALS.choices.viewer.scale;

  err = display_set_handlers(v);
  if (err)
    goto Failure;

  list_add_to_head(&list_anchor, &v->list);

  *new_viewer = v;

  return result_OK;


NoMem:

  err = result_OOM;

  goto Failure;


Failure:

  if (w != wimp_ICON_BAR)
    window_delete_cloned(w);

  free(v);

  result_report(err);

  return err;
}

/* Disposes of things associated with a viewer, e.g. after an edit. */
static void viewer_dispose(viewer_t *v)
{
  /* Abandon clipboard */
  if (GLOBALS.clipboard_viewer == v)
    GLOBALS.clipboard_viewer = NULL;

#ifdef EYE_ZONES
  zones_destroy(v->zones);
  v->zones = NULL;
#endif
}

static void viewer_reset(viewer_t *v)
{
  viewer_dispose(v);

  /* Force the viewer to take the user-defined default scale. */
  if (GLOBALS.choices.viewer.scale != viewerscale_PRESERVE)
    v->scale.cur = v->scale.prev = GLOBALS.choices.viewer.scale;
}

void viewer_destroy(viewer_t *doomed)
{
  viewer_reset(doomed);

  display_release_handlers(doomed);

  list_remove(&list_anchor, &doomed->list);

  /* Delete the window */
  window_delete_cloned(doomed->main_w);

  /* viewer_unload should have been called at this point */

  /* Delete the viewer */
  free(doomed);

  /* Update any title bars containing cloned viewer counters */
  /* FIXME: If this could avoid refreshing any unaffected title bars, that
   *        would be nice. Perhaps with a _CLONED/_UNCLONED image event? */
  refresh_all_titles();
}

viewer_t *viewer_find(wimp_w w)
{
  return (viewer_t *) list_find(&list_anchor,
                                 offsetof(viewer_t, main_w),
                           (int) w);
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  const char *file_name;
  bits        load;
  bits        exec;
  viewer_t   *result;
}
find_by_attrs_args;

static int find_by_attrs_callback(list_t *e, void *opaque)
{
  find_by_attrs_args *args;
  viewer_t           *v;
  image_t            *i;

  args = opaque;

  v = (viewer_t *) e;

  i = v->drawable->image;

  if (strcmp(i->file_name, args->file_name) == 0 &&
      i->source.load == args->load &&
      i->source.exec == args->exec)
  {
    args->result = v;
    return -1; /* halt the list walk */
  }

  return 0;
}

viewer_t *viewer_find_by_attrs(const char *file_name, bits load, bits exec)
{
  find_by_attrs_args args;

  args.file_name = file_name;
  args.load      = load;
  args.exec      = exec;
  args.result    = NULL;

  list_walk(&list_anchor, find_by_attrs_callback, &args);

  return args.result;
}

/* ----------------------------------------------------------------------- */

int viewer_count_clones(viewer_t *v)
{
#if 1 /* quicker method */
  return v->drawable->image->refcount;
#else
  image *i;
  int    c;

  i = v->drawable->image;

  c = 0;
  for (v = first; v != NULL; v = v->next)
    if (v->drawable->image == i)
      c++;

  return c;
#endif
}

/* ----------------------------------------------------------------------- */

/* Call the specified function for every viewer window. */
void viewer_map(viewer_map_callback *fn, void *opaque)
{
  /* Note that the callback signatures are identical, so we can cast. */

  list_walk(&list_anchor, (list_walk_callback_t *) fn, opaque);
}

/* ----------------------------------------------------------------------- */

/* Like viewer_map, but for a specific image. */
void viewer_map_for_image(image_t             *image,
                          viewer_map_callback *fn,
                          void                *opaque)
{
  list_t *e;
  list_t *next;

  /* Be careful about linked list walking in case the callback is destroying
   * the objects we're iterating over. */
  for (e = list_anchor.next; e != NULL; e = next)
  {
    viewer_t *v;

    next = e->next;

    v = (viewer_t *) e;

    if (v->drawable->image == image)
      fn(v, opaque);
  }
}

/* ----------------------------------------------------------------------- */

int viewer_get_count(void)
{
  /* FIXME: This is not the actual count! It doesn't matter at the moment as
   * this value is only used to determine whether to shade menu entries. */
  return (list_anchor.next == NULL) ? 0 : 1;
}

/* ----------------------------------------------------------------------- */

static struct
{
  struct
  {
    osspriteop_area      *area;
    osspriteop_header    *header;  /* Used when plotting sprites. */
    osspriteop_trans_tab *trans_tab;
    os_factors            factors;
  }
  checker;
}
LOCALS;

void viewer_mode_change(void)
{
  const int          log2scale = 4; /* checker scale factor */

  osspriteop_area   *area;
  osspriteop_header *header;
  os_mode            mode;
  int                image_xeig, image_yeig;
  int                screen_xeig, screen_yeig;
  os_factors        *scaled_factors;

  /* colours */

  if (LOCALS.checker.trans_tab)
    flex_free((flex_ptr) &LOCALS.checker.trans_tab); 

  area   = window_get_sprite_area();
  header = osspriteop_select_sprite(osspriteop_NAME,
                                    area,
                    (osspriteop_id) "checker");
  if (sprite_colours(&area, header, &LOCALS.checker.trans_tab))
    return; /* NoMem */

  LOCALS.checker.area   = area;
  LOCALS.checker.header = header;

  /* scaling */

  sprite_info(area, header, NULL, NULL, NULL, &mode, NULL);
  read_mode_vars(mode, &image_xeig, &image_yeig, NULL);
  read_current_mode_vars(&screen_xeig, &screen_yeig, NULL);

  scaled_factors = &LOCALS.checker.factors;
  scaled_factors->xmul = image_xeig << log2scale;
  scaled_factors->ymul = image_yeig << log2scale;
  scaled_factors->xdiv = screen_xeig;
  scaled_factors->ydiv = screen_yeig;
}

/* Clears the graphics window. */
static void clg(int x, int y, os_colour colour)
{
  static unsigned int try_flags = 3;

  if (colour == os_COLOUR_TRANSPARENT)
  {
    /* Try to use the recent PlotTiledSprite SpriteOp. If that fails never
     * attempt it again. Do the same for Tinct. */
    if (try_flags & 1)
    {
      os_error *e;

      e = xosspriteop_plot_tiled_sprite(osspriteop_PTR,
                                        LOCALS.checker.area,
                        (osspriteop_id) LOCALS.checker.header,
                                        x, y,
                                        osspriteop_GIVEN_WIDE_ENTRIES,
                                       &LOCALS.checker.factors,
                                        LOCALS.checker.trans_tab);
      if (e)
        try_flags &= ~1;
      else
        return;
    }
    
    if (try_flags & 2)
    {
      _kernel_oserror *e;

#define Tinct_PlotScaled (0x57243)
      e = _swix(Tinct_PlotScaled, _INR(2,6)|_IN(7),
                                  LOCALS.checker.header,
                                  x, y,
                                  32, 32,
                                  0x3C);
      if (e)
        try_flags &= ~2;
      else
        return;
    }
  }

  colourtrans_set_gcol(colour,
                       colourtrans_SET_BG_GCOL,
                       os_ACTION_OVERWRITE,
                       NULL);
  os_writec(os_VDU_CLG);
}

/* Draws a background for transparent/masked bitmaps and vector images. */
static void clg_draw(wimp_draw *draw, viewer_t *viewer, int x, int y)
{
  NOT_USED(draw);

  clg(x, y, viewer->background.colour);
}

/* Draw the window background by filling in the regions surrounding the
 * opaque bitmaps. This avoids flicker. */
static void stage_draw(wimp_draw *draw, viewer_t *viewer, int x, int y)
{
  static const os_colour *colours[stageboxkind__LIMIT] =
  {
    &GLOBALS.choices.viewer.stage.pasteboard.colour,
    &GLOBALS.choices.viewer.stage.stroke.colour,
    &GLOBALS.choices.viewer.stage.margin.colour,
    NULL, /* Content */
    &GLOBALS.choices.viewer.stage.shadow.colour,
  };

  int i;

  for (i = 0; i < (int) viewer->background.stage.nboxes; i++)
  {
    const stagebox_t *sb = &viewer->background.stage.boxes[i];
    os_box            box;
    os_box            clip;
    const os_colour  *pcolour;
    os_colour         colour;

    if ((viewer->drawable->flags & drawable_FLAG_DRAW_BG) == 0 &&
        sb->kind == stageboxkind_CONTENT) /* i == 0 would also work */
      continue;

    box_translated(&sb->box, x, y, &box);
    box_intersection(&draw->clip, &box, &clip);
    if (box_is_empty(&clip))
      continue;

    screen_clip(&clip);

    pcolour = colours[sb->kind];
    colour = (pcolour) ? *pcolour : viewer->background.colour;
    clg(x, y, colour);
  }
  screen_clip(&draw->clip);
}

static os_colour bgcolour_from_type(bits file_type)
{
  switch (file_type)
  {
  case gif_FILE_TYPE:      return GLOBALS.choices.gif.background;
  case osfile_TYPE_DRAW:   return GLOBALS.choices.drawfile.background;
  case png_FILE_TYPE:      return GLOBALS.choices.png.background;
  case jpeg_FILE_TYPE:     return GLOBALS.choices.jpeg.background;
  case artworks_FILE_TYPE: return GLOBALS.choices.artworks.background;
  default:
  case osfile_TYPE_SPRITE: return GLOBALS.choices.sprite.background;
  }
}

void viewer_set_extent_from_box(viewer_t *viewer, const os_box *dimensions)
{
  int    xpix, ypix;
  os_box rounded;
  int    iw,ih;
  os_box extent;
  int    sw,sh;
  int    minimum_size;
  int    x,y;

  /* Read the size of a pixel in OS units. */
  read_current_pixel_size(&xpix, &ypix);

  /* Round specified dimensions to whole pixel OS units. */
  rounded.x0 = dimensions->x0 & ~(xpix - 1);
  rounded.y0 = dimensions->y0 & ~(ypix - 1);
  rounded.x1 = dimensions->x1 & ~(xpix - 1);
  rounded.y1 = dimensions->y1 & ~(ypix - 1);

  /* Remember the image extent. */
  viewer->imgdims = rounded;

  iw = rounded.x1 - rounded.x0;
  ih = rounded.y1 - rounded.y0;

  if (GLOBALS.choices.viewer.size == viewersize_FIT_TO_SCREEN)
  {
    int fixed_x, fixed_y;

    read_max_visible_area(viewer->main_w, &sw, &sh);
    if (!GLOBALS.choices.viewer.cover_icon_bar)
      sh -= read_icon_bar_unobscured();

    stage_get_fixed(&viewer->background.stage.config, &fixed_x, &fixed_y);

    /* Enforce a minimum size. */
    sw = MAX(sw, iw + fixed_x);
    sh = MAX(sh, ih + fixed_y);
  }
  else
  {
    sw = iw;
    sh = ih;
  }

  extent.x0 = 0;
  extent.y0 = 0;
  extent.x1 = sw;
  extent.y1 = sh;

  /* Note: We can't do this on a mode change. */
  minimum_size = window_set_extent2(viewer->main_w,
                                    extent.x0,
                                    extent.y0,
                                    extent.x1,
                                    extent.y1);

  /* Re-read the actual extent if it hit minimum. */
  if (minimum_size)
  {
    wimp_window_info info;

    info.w = viewer->main_w;
    wimp_get_window_info(&info);

    sw = info.extent.x1 - info.extent.x0;
    sh = info.extent.y1 - info.extent.y0;

    extent = info.extent;
  }

  /* Remember the window extent. */
  viewer->extent = extent;

  /* Select a background filling mode. */
  if (GLOBALS.choices.viewer.size == viewersize_FIT_TO_SCREEN)
  {
    viewer->background.draw    = stage_draw;

    stage_generate(&viewer->background.stage.config,
                    sw, sh,
                    iw, ih,
                    NULL, NULL,
                    viewer->background.stage.boxes,
                   &viewer->background.stage.nboxes);

    viewer->imgbox = viewer->background.stage.boxes[0].box;
  }
  else
  {
    /* Calculate where to draw the image (by default). */
    x = (sw - iw) / 2;
    y = (sh - ih) / 2;
  
    /* Round the result to the size of a whole pixel to avoid redraw glitches. */
    x &= ~(xpix - 1);
    y &= ~(ypix - 1);

    /* Remember the image extents. */
    viewer->imgbox = (os_box) { x, y, x+iw, y+ih };

    if (minimum_size || viewer->drawable->flags & drawable_FLAG_DRAW_BG)
    {
      viewer->background.draw    = clg_draw;
    }
    else
    {
      /* Opaque images won't redraw their background so to avoid flicker. */
      viewer->background.draw    = NULL;
    }
  }

  if (viewer->background.draw)
  {
    /* If the image specifies its own background colour then use it. */
    if (viewer->image->background_colour == os_COLOUR_TRANSPARENT)
      viewer->background.colour = bgcolour_from_type(viewer->image->source.file_type);
    else
      viewer->background.colour = viewer->image->background_colour;
  }
}

static void scale_for_screen(viewer_t *viewer)
{
  int s;

  s = viewer_scaledlg_fit_to_screen(viewer);
  if (s > 100)
    s = 100;

  viewer->scale.cur = viewer->scale.prev = s;
}

/* Called when a change has occurred. */
void viewer_update(viewer_t *viewer, viewer_update_flags flags)
{
  drawable_t *d;

  d = viewer->drawable;

  /* If the image format has changed we need to throw away the previous
   * drawable. */

  if (flags & viewer_UPDATE_FORMAT)
    drawable_reset(d);

  if ((flags & viewer_UPDATE_COLOURS) && d->methods.update_colours)
    d->methods.update_colours(d);

  if (flags & (viewer_UPDATE_SCALING | viewer_UPDATE_EXTENT))
  {
    os_box     dimensions;
    os_factors factors;

    if (viewer->scale.cur == viewerscale_FIT_TO_SCREEN)
      scale_for_screen(viewer);

    /* Convert the percentage current scale into an os_factors. */

    os_factors_from_ratio(&factors, viewer->scale.cur, SCALE_100PC);

    if (flags & viewer_UPDATE_SCALING)
    {
      d->methods.update_scaling(d, &factors);
      update_viewer_title(viewer);
    }

    if (flags & viewer_UPDATE_EXTENT)
    {
      d->methods.get_dimensions(d, &factors, &dimensions);
      viewer_set_extent_from_box(viewer, &dimensions);
    }
  }

  /* Redraw the image only */
  if (flags & viewer_UPDATE_CONTENT)
  {
    wimp_force_redraw(viewer->main_w, viewer->imgbox.x0,
                                      viewer->imgbox.y0,
                                      viewer->imgbox.x1,
                                      viewer->imgbox.y1);
  }

  /* Redraw the whole window */
  if (flags & viewer_UPDATE_REDRAW)
  {
    window_redraw(viewer->main_w);
  }
}

/* ----------------------------------------------------------------------- */

static int update_all_callback(viewer_t *viewer, void *opaque)
{
  viewer_update(viewer, (unsigned int) opaque);

  return 0;
}

void viewer_update_all(viewer_update_flags flags)
{
  viewer_map(update_all_callback, (void *) flags);
}

/* ----------------------------------------------------------------------- */

void viewer_open(viewer_t *viewer)
{
  window_restore(viewer->main_w,
                &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);
}

/* ----------------------------------------------------------------------- */

struct update_image_args
{
  viewer_update_flags flags;
};

static int update_image_callback(viewer_t *viewer, void *opaque)
{
  struct update_image_args *args = opaque;

  viewer_update(viewer, args->flags);

  return 0;
}

static int capture_position_callback(viewer_t *viewer, void *opaque)
{
  NOT_USED(opaque);

  window_capture(viewer->main_w,
                &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  return 0;
}

static int restore_position_callback(viewer_t *viewer, void *opaque)
{
  NOT_USED(opaque);

  window_restore(viewer->main_w,
                &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  return 0;
}

static int reset_callback(viewer_t *viewer, void *opaque)
{
  NOT_USED(opaque);

  viewer_dispose(viewer);

  return 0;
}

static int set_titles_callback(viewer_t *viewer, void *opaque)
{
  NOT_USED(opaque);

  update_viewer_title(viewer);

  return 0;
}

/* Called by imageobserver when a registered image changes. */
static void image_changed_callback(image_t             *image,
                                   imageobserver_change change,
                                   imageobserver_data  *data,
                                   void                *opaque)
{
  struct update_image_args args;

  NOT_USED(opaque);

  switch (change)
  {
  case imageobserver_CHANGE_ABOUT_TO_MODIFY:
    /* The image is about to be modified, possibly changing size, so capture
     * the window position as it is now. */
    viewer_map_for_image(image, capture_position_callback, NULL);
    break;

  case imageobserver_CHANGE_PREVIEW:
    /* The preview image has been updated, so just redraw. */
    args.flags = viewer_UPDATE_CONTENT;
    viewer_map_for_image(image, update_image_callback, &args);
    break;

  case imageobserver_CHANGE_MODIFIED:
    /* The image has been modified (which might mean anything) so reset the
     * image object's transient stuff (like zones) and reset the scale. */
    viewer_map_for_image(image, reset_callback, NULL);

    /* Convert the image change into a viewer update. */

    args.flags = 0;

    if (data->modified.flags & image_MODIFIED_DATA)
      args.flags |= viewer_UPDATE_COLOURS |
                    viewer_UPDATE_SCALING |
                    viewer_UPDATE_EXTENT  |
                    viewer_UPDATE_REDRAW;

    if (data->modified.flags & image_MODIFIED_FORMAT)
      args.flags |= viewer_UPDATE_ALL;

    viewer_map_for_image(image, update_image_callback, &args);

    viewer_map_for_image(image, restore_position_callback, NULL);
    break;

  case imageobserver_CHANGE_SAVED:
    viewer_map_for_image(image, set_titles_callback, NULL);
    break;
  }
}

/* ----------------------------------------------------------------------- */

void viewer_clone(viewer_t *source)
{
  result_t          err;
  viewer_t         *v;
  wimp_window_state state;

  err = viewer_clone_from_window(source->main_w, &v);
  if (err)
    goto Failure;

  state.w = source->main_w;
  wimp_get_window_state(&state);

  state.w = v->main_w;
  state.visible.y1 -= 48;
  state.visible.y0 -= 48;
  state.next = wimp_TOP;
  wimp_open_window((wimp_open *) &state);

  wimp_set_caret_position(v->main_w, wimp_ICON_WINDOW, 0, 0, -1, -1);

  refresh_all_titles();

  return;


Failure:
  /* This is the highest level so tell the user of the failure and return. */
  result_report(err);

  return;
}

result_t viewer_clone_from_window(wimp_w w, viewer_t **pviewer)
{
  result_t  err;
  viewer_t *v;
  viewer_t *newv;

  v = viewer_find(w);
  if (v == NULL)
  {
    err = result_PRIVATEEYE_VIEWER_NOT_FOUND;
    goto Failure;
  }

  err = viewer_create(&newv);
  if (err)
    goto Failure;

  err = drawable_clone(v->drawable, &newv->drawable);
  if (err)
    goto Failure;

  newv->image = v->image;
  image_addref(v->image);

  viewer_update(newv, viewer_UPDATE_ALL);

#ifdef EYE_ZONES
  newv->zones = zones_create(newv->drawable->image);
#endif

  /* Note that zones can validly be NULL (e.g. for vectors). */

  /* Watch for changes. */
  imageobserver_register(newv->drawable->image,
                         image_changed_callback,
                         NULL);

  *pviewer = newv;

  return result_OK;


Failure:
  if (newv)
  {
    drawable_destroy(newv->drawable);
    viewer_destroy(newv);
  }

  return err;
}

/* ----------------------------------------------------------------------- */

/* Loads the specified file into an existing viewer block.
 *
 * Remember that the existing block may already have structures in it
 * associated with a previous image.
 */

osbool viewer_load(viewer_t *viewer,
             const char     *file_name,
                   bits      load,
                   bits      exec,
                   osbool    unsafe,
                   osbool    template)
{
  result_t    rc;
  image_t    *image = NULL;
  drawable_t *drawable;
  osbool      r;
  wimp_caret  caret;

  hourglass_on();

  /* Remember current window position for when we position window later. */
  window_capture(viewer->main_w,
                &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  /* Get rid of any previous stuff. */
  viewer_reset(viewer);

  /* Is it in the cache? */

  rc = imagecache_get(GLOBALS.cache,
                     &GLOBALS.choices.image,
                      file_name,
                      load,
                      exec,
                     &image);
  if (rc)
    goto Failure;

  if (unsafe)
  {
    image->file_name[0] = '\0';
    image->flags |= image_FLAG_MODIFIED;
  }

  if (template)
    strcpy(image->file_name, str_leaf(file_name));

  rc = drawable_create(image, &drawable);
  if (rc)
    goto Failure;

  viewer->image    = image;
  viewer->drawable = drawable;

  viewer_update(viewer, viewer_UPDATE_ALL);

#ifdef EYE_ZONES
  /* Now the viewer block is ready we can create the zones. */
  viewer->zones = zones_create(image);
  /* viewer->zones may be NULL on return. */
#endif

  /* Watch for changes. */
  imageobserver_register(image, image_changed_callback, NULL);

  /* If we have the focus - inform the image. */

  wimp_get_caret_position(&caret);
  if (caret.w == viewer->main_w)
    image_focus(image);

  r = FALSE; /* success */

Exit:

  result_report(rc);

  hourglass_off();

  return r;


Failure:

  imagecache_dispose(GLOBALS.cache, image);

  r = TRUE;

  goto Exit;
}

osbool viewer_save(viewer_t *viewer, const char *file_name, osbool unsafe)
{
  if (viewer->image->methods.save(&GLOBALS.choices.image,
                                   viewer->image,
                                   file_name))
  {
    return TRUE; /* failure */
  }

  if (!unsafe)
  {
    strcpy(viewer->image->file_name, file_name);
    image_saved(viewer->image); /* will cause viewer title update */
  }

  return FALSE; /* success */
}

void viewer_unload(viewer_t *viewer)
{
  /* FIXME: Only if we have the focus... */
  image_defocus(viewer->image);

  imageobserver_deregister(viewer->image, image_changed_callback, NULL);

#ifdef EYE_ZONES
  zones_destroy(viewer->zones);
  viewer->zones = NULL;
#endif

  drawable_destroy(viewer->drawable);
  viewer->drawable = NULL;

  imagecache_dispose(GLOBALS.cache, viewer->image);
  viewer->image = NULL;
}

/* ----------------------------------------------------------------------- */

int viewer_query_unload(viewer_t *viewer)
{
  if (viewer->image->flags & image_FLAG_MODIFIED &&
      viewer_count_clones(viewer) == 1)
  {
    switch (dcs_quit_dcs_query("dcs.modified"))
    {
    case dcs_quit_DISCARD:
      return 1;

    case dcs_quit_CANCEL:
      return 0;

    case dcs_quit_SAVE:
      dialogue_show(viewer_savedlg);
      return 0;
    }
  }

  return 1;
}

/* ----------------------------------------------------------------------- */

static int close_all_callback(viewer_t *viewer, void *opaque)
{
  NOT_USED(opaque);

  viewer_unload(viewer);
  viewer_destroy(viewer);

  return 0;
}

void viewer_close_all(void)
{
  hourglass_on();

  viewer_map(close_all_callback, NULL);

  hourglass_off();
}

/* ----------------------------------------------------------------------- */

static void count_edited_callback(image_t *image, void *opaque)
{
  int *nedited = opaque;

  if (image->flags & image_FLAG_MODIFIED)
    (*nedited)++;
}

int viewer_count_edited(void)
{
  int nedited;

  /* Count the number of edited images. */

  nedited = 0;
  image_map(count_edited_callback, &nedited);

  return nedited;
}

/* ----------------------------------------------------------------------- */

result_t viewer_choices_updated(const choices_group *group)
{
  NOT_USED(group);

  viewer_update_all(viewer_UPDATE_ALL);

  return result_OK;
}
