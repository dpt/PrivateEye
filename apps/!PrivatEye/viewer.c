/* --------------------------------------------------------------------------
 *    Name: viewer.c
 * Purpose: Viewer block handling
 * Version: $Id: viewer.c,v 1.66 2009-11-29 23:18:37 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "swis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/colourtrans.h"
#include "oslib/hourglass.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/drawfile.h"
#include "oslib/jpeg.h"

#include "appengine/datastruct/array.h"
#include "appengine/dialogues/dcs-quit.h"
#include "appengine/geom/box.h"
#include "appengine/app/choices.h"
#include "appengine/graphics/drawable.h"
#include "appengine/base/errors.h"
#include "appengine/graphics/image-observer.h"
#include "appengine/graphics/image.h"
#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"
#include "appengine/vdu/screen.h"
#include "appengine/base/strings.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/window.h"

#include "display.h"
#include "globals.h"
#include "privateeye.h"
#include "save.h"
#include "scale.h"
#include "viewer.h"
#include "zones.h"
#include "imagecache.h"

static list_t list_anchor = { NULL };

/* ----------------------------------------------------------------------- */

static void set_view_title(viewer *viewer)
{
  char file_name[256];
  char percentage[12];
  char buf[256];
  int  nclones;

  sprintf(file_name, "%.*s", (int) sizeof(file_name), viewer->image->file_name);
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

static int refresh_all_titles_callback(viewer *viewer, void *arg)
{
  NOT_USED(arg);

  set_view_title(viewer);

  return 0;
}

static void refresh_all_titles(void)
{
  viewer_map(refresh_all_titles_callback, NULL);
}

/* ----------------------------------------------------------------------- */

error viewer_create(viewer **new_viewer)
{
  error   err;
  viewer *v;
  wimp_w  w = wimp_ICON_BAR;

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

  if (GLOBALS.choices.viewer.scale == viewerscale_PRESERVE)
    /* 'Preserve' means take the last scale. We have no previous, so use
     * 100%. */
    v->scale.cur = v->scale.prev = 100;
  else
    v->scale.cur = v->scale.prev = GLOBALS.choices.viewer.scale;

  err = display__set_handlers(v);
  if (err)
    goto Failure;

  list__add_to_head(&list_anchor, &v->list);

  *new_viewer = v;

  return error_OK;


NoMem:

  err = error_OOM;

  goto Failure;


Failure:

  if (w != wimp_ICON_BAR)
    window_delete_cloned(w);

  free(v);

  error__report(err);

  return err;
}

/* Disposes of things associated with a viewer, e.g. after an edit. */
static void viewer_dispose(viewer *v)
{
  /* Abandon clipboard */
  if (GLOBALS.clipboard_viewer == v)
    GLOBALS.clipboard_viewer = NULL;

#ifdef EYE_ZONES
  zones_destroy(v->zones);
  v->zones = NULL;
#endif
}

static void viewer_reset(viewer *v)
{
  viewer_dispose(v);

  /* Force the viewer to take the user-defined default scale. */
  if (GLOBALS.choices.viewer.scale != viewerscale_PRESERVE)
    v->scale.cur = v->scale.prev = GLOBALS.choices.viewer.scale;
}

void viewer_destroy(viewer *doomed)
{
  viewer_reset(doomed);

  display__release_handlers(doomed);

  list__remove(&list_anchor, &doomed->list);

  /* Delete the window */
  window_delete_cloned(doomed->main_w);

  /* viewer_unload should have been called at this point */

  /* Delete the viewer */
  free(doomed);

  /* Update any title bars containing cloned viewer counters */
  /* FIXME: If this could avoid refreshing any unaffected title bars, that
   *        would be nice. */
  refresh_all_titles();
}

viewer *viewer_find(wimp_w w)
{
  return (viewer *) list__find(&list_anchor,
                                offsetof(viewer, main_w),
                          (int) w);
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  const char *file_name;
  bits        load;
  bits        exec;
  viewer     *result;
}
find_by_attrs_args;

static int find_by_attrs_callback(list_t *e, void *arg)
{
  find_by_attrs_args *args;
  viewer             *v;
  image              *i;

  args = arg;

  v = (viewer *) e;

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

viewer *viewer_find_by_attrs(const char *file_name, bits load, bits exec)
{
  find_by_attrs_args args;

  args.file_name = file_name;
  args.load      = load;
  args.exec      = exec;
  args.result    = NULL;

  list__walk(&list_anchor, find_by_attrs_callback, &args);

  return args.result;
}

/* ----------------------------------------------------------------------- */

int viewer_count_clones(viewer *v)
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

void viewer_map(viewer_map_callback *fn, void *arg)
{
  /* Note that the callback signatures are identical, so we can cast. */

  list__walk(&list_anchor, (list__walk_callback *) fn, arg);
}

/* ----------------------------------------------------------------------- */

/* Like viewer_map, but for a specific image. */
void viewer_map_for_image(image *image, viewer_map_callback *fn, void *arg)
{
  list_t *e;
  list_t *next;

  /* be careful about linked list walking in case the caller is destroying
   * the objects we're iterating through */
  for (e = list_anchor.next; e != NULL; e = next)
  {
    viewer *v;

    next = e->next;

    v = (viewer *) e;

    if (v->drawable->image == image)
      fn(v, arg);
  }
}

/* ----------------------------------------------------------------------- */

int viewer_get_count(void)
{
  return (list_anchor.next == NULL) ? 0 : 1; /* FIXME: Not actual count! */
}

/* ----------------------------------------------------------------------- */

static void fill_redraw_rect_prepare(viewer *viewer)
{
  osspriteop_area *area;

  area = window_get_sprite_area();
  viewer->background.header = osspriteop_select_sprite(osspriteop_NAME,
                                                       area,
                                      (osspriteop_id) "checker");
}

#define Tinct_PlotScaled 0x57243

/* Draws a background for transparent/masked bitmaps and vector images. */
static void fill_redraw_rect(wimp_draw *draw, viewer *viewer, int x, int y)
{
  NOT_USED(draw);

  if (viewer->background.colour == os_COLOUR_TRANSPARENT)
  {
    _kernel_oserror *e;

    e = _swix(Tinct_PlotScaled, _INR(2,6)|_IN(7),
              viewer->background.header, x, y, 32, 32, 0x3C);
    if (e == NULL)
      return;
  }

  colourtrans_set_gcol(viewer->background.colour,
                       colourtrans_SET_BG_GCOL,
                       os_ACTION_OVERWRITE,
                       NULL);

  os_writec(os_VDU_CLG);
}

static void draw_edge(wimp_draw *draw,
                      viewer    *viewer,
                      int        x,
                      int        y,
                const os_box    *box)
{
  os_box clip;

  box__intersection(&draw->clip, box, &clip);
  if (box__is_empty(&clip))
    return;

  screen_clip(&clip);
  fill_redraw_rect(draw, viewer, x, y);
}

/* Draws a background *around* opaque bitmaps. Draws only the edge boxes to
 * avoid flicker. */
static void draw_edges_only(wimp_draw *draw, viewer *viewer, int x, int y)
{
  os_box *extent;
  os_box *imgbox;
  os_box  box;

  extent = &viewer->extent;
  imgbox = &viewer->imgbox;

  /* Draw top, bottom, left, right */

  box.x0 = extent->x0 + x;
  box.y0 = imgbox->y1 + y;
  box.x1 = extent->x1 + x;
  box.y1 = extent->y1 + y;
  draw_edge(draw, viewer, x,y, &box);

  /* top and bottom share x coordinates */
  box.y0 = extent->y0 + y;
  box.y1 = imgbox->y0 + y;
  draw_edge(draw, viewer, x,y, &box);

  /* left and bottom share x0 only */
  box.y0 = imgbox->y0 + y;
  box.x1 = imgbox->x0 + x;
  box.y1 = imgbox->y1 + y;
  draw_edge(draw, viewer, x,y, &box);

  /* right and left share y coordinates */
  box.x0 = imgbox->x1 + x;
  box.x1 = extent->x1 + x;
  draw_edge(draw, viewer, x,y, &box);

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

void viewer_set_extent_from_box(viewer *viewer, const os_box *box)
{
  int iw,ih;
  os_box extent;
  int sw,sh;
  int minimum_size;
  int xpix, ypix;

  iw = box->x1 - box->x0;
  ih = box->y1 - box->y0;

  if (GLOBALS.choices.viewer.size == viewersize_FIT_TO_SCREEN)
  {
    read_max_visible_area(viewer->main_w, &sw, &sh);

    if (!GLOBALS.choices.viewer.cover_icon_bar)
      sh -= IconBarVisible;

    /* enforce a minimum size */
    if (sw < iw)
      sw = iw;
    if (sh < ih)
      sh = ih;

    extent.x0 = box->x0;
    extent.y0 = box->y0;
    extent.x1 = box->x0 + sw;
    extent.y1 = box->y0 + sh;
  }
  else
  {
    sw = iw;
    sh = ih;

    extent = *box;
  }

  /* note: can't do this on a mode change */
  minimum_size = window_set_extent2(viewer->main_w,
                                    extent.x0,
                                    extent.y0,
                                    extent.x1,
                                    extent.y1);

  /* re-read the actual extent if it hit minimum */
  if (minimum_size)
  {
    wimp_window_info info;

    info.w = viewer->main_w;
    wimp_get_window_info(&info);

    extent = info.extent;
  }

  /* remember the extent */
  viewer->extent = extent;

  /* set where to draw the image */
  viewer->x = (sw - iw) / 2;
  viewer->y = (sh - ih) / 2;

  /* Read the size of a pixel in OS units */
  read_current_pixel_size(&xpix, &ypix);

  /* round to a whole pixel to avoid redraw glitches */
  viewer->x &= ~(xpix - 1);
  viewer->y &= ~(ypix - 1);

  /* remember the image extent */
  viewer->imgbox.x0 = viewer->x;
  viewer->imgbox.y0 = viewer->y;
  viewer->imgbox.x1 = viewer->x + iw;
  viewer->imgbox.y1 = viewer->y + ih;

  if (viewer->drawable->flags & drawable_FLAG_DRAW_BG)
  {
    viewer->background.prepare = fill_redraw_rect_prepare;
    viewer->background.draw    = fill_redraw_rect;
  }
  else if (minimum_size || GLOBALS.choices.viewer.size == viewersize_FIT_TO_SCREEN)
  {
    viewer->background.prepare = fill_redraw_rect_prepare;
    viewer->background.draw    = draw_edges_only;
  }
  else
  {
    viewer->background.prepare = NULL;
    viewer->background.draw    = NULL;
  }

  if (viewer->background.draw)
  {
    /* if the image specifies its own background colour then use it */
    if (viewer->image->background_colour == os_COLOUR_TRANSPARENT)
      viewer->background.colour = bgcolour_from_type(viewer->image->source.file_type);
    else
      viewer->background.colour = viewer->image->background_colour;
  }
}

/* FIXME: Largely the same as fit_to_screen in scale.c */
static void scale_for_screen(viewer *viewer)
{
  int w,h;
  int s;

  read_max_visible_area(viewer->main_w, &w, &h);

  s = scale_for_box(viewer->drawable, w, h);

  viewer->scale.cur = viewer->scale.prev = s;
}

void viewer_update(viewer *viewer, viewer_update_flags flags)
{
  drawable *d;

  d = viewer->drawable;

  /* if the image format has changed, we need to throw away the previous
   * drawable */

  if (flags & viewer_UPDATE_FORMAT)
    drawable_reset(d);

  if ((flags & viewer_UPDATE_COLOURS) && d->methods.update_colours)
    d->methods.update_colours(d);

  if (flags & (viewer_UPDATE_SCALING | viewer_UPDATE_EXTENT))
  {
    os_box     box;
    os_factors factors;

    if (viewer->scale.cur == viewerscale_FIT_TO_SCREEN)
      scale_for_screen(viewer);

    /* convert the percentage current scale into an os_factors */

    os_factors_from_ratio(&factors, viewer->scale.cur, SCALE_100PC);

    if (flags & viewer_UPDATE_SCALING)
    {
      d->methods.update_scaling(d, &factors);
      set_view_title(viewer);
    }

    if (flags & viewer_UPDATE_EXTENT)
    {
      d->methods.get_dimensions(d, &factors, &box);
      viewer_set_extent_from_box(viewer, &box);
    }
  }

  if (flags & viewer_UPDATE_REDRAW)
    window_redraw(viewer->main_w);
}

/* ----------------------------------------------------------------------- */

static int update_all_callback(viewer *viewer, void *arg)
{
  viewer_update(viewer, (unsigned int) arg);

  return 0;
}

void viewer_update_all(viewer_update_flags flags)
{
  viewer_map(update_all_callback, (void *) flags);
}

/* ----------------------------------------------------------------------- */

void viewer_open(viewer *viewer)
{
  window_restore(viewer->main_w, &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);
}

/* ----------------------------------------------------------------------- */

struct update_image_args
{
  viewer_update_flags flags;
};

static int update_image_callback(viewer *viewer, void *arg)
{
  struct update_image_args *args = arg;

  viewer_update(viewer, args->flags);

  return 0;
}

static int capture_position_callback(viewer *viewer, void *arg)
{
  NOT_USED(arg);

  window_capture(viewer->main_w, &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  return 0;
}

static int restore_position_callback(viewer *viewer, void *arg)
{
  NOT_USED(arg);

  window_restore(viewer->main_w, &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  return 0;
}

static int reset_callback(viewer *viewer, void *arg)
{
  NOT_USED(arg);

  viewer_dispose(viewer);

  return 0;
}

/* Call by the imageobserver when a registered image changes. */
static void image_changed_callback(image                *image,
                                   imageobserver_change  change,
                                   imageobserver_data   *data)
{
  struct update_image_args args;

  switch (change)
  {
  case imageobserver_CHANGE_ABOUT_TO_MODIFY:
    /* The image is about to be modified, possibly changing size, so capture
     * the window position as it is now. */
    viewer_map_for_image(image, capture_position_callback, NULL);
    break;

  case imageobserver_CHANGE_PREVIEW:
    /* The preview image has been updated, so just redraw. */
    args.flags = viewer_UPDATE_REDRAW;
    viewer_map_for_image(image, update_image_callback, &args);
    break;

  case imageobserver_CHANGE_MODIFIED:
    /* The image has been modified, which might mean anything, so reset the
     * image object's transient stuff (like zones and reset the scale). */
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
  }
}

/* ----------------------------------------------------------------------- */

void viewer_clone(viewer *source)
{
  error             err;
  viewer           *v;
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
  error__report(err);

  return;
}

error viewer_clone_from_window(wimp_w w, viewer **pviewer)
{
  error   err;
  viewer *v;
  viewer *newv;

  v = viewer_find(w);
  if (v == NULL)
  {
    err = error_PRIVATEEYE_VIEWER_NOT_FOUND;
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

  /* watch for changes */
  imageobserver_register(newv->drawable->image, image_changed_callback);

  *pviewer = newv;

  return error_OK;


Failure:
  if (newv)
  {
    drawable_destroy(newv->drawable);
    viewer_destroy(newv);
  }

  return err;
}

/* ----------------------------------------------------------------------- */

/* Loads the specified file into an existing eye block.
 *
 * Remember that the existing block may already have structures in it
 * associated with the previous image.
 */

osbool viewer_load(viewer     *viewer,
                   const char *file_name,
                   bits        load,
                   bits        exec)
{
  error       err;
  image      *image = NULL;
  drawable   *drawable;
  osbool      r;
  wimp_caret  caret;

  hourglass_on();

  /* remember current window position for when we position window later */
  window_capture(viewer->main_w, &viewer->capture,
                 GLOBALS.choices.viewer.cover_icon_bar);

  /* get rid of any previous stuff */
  viewer_reset(viewer);

  /* is it in the cache? */

  err = imagecache_get(file_name, load, exec, &image);
  if (err)
    goto Failure;

  if (image == NULL)
  {
    /* it's not in the cache */

    image = image_create_from_file(&GLOBALS.choices.image,
                                    file_name,
                                    (load >> 8) & 0xfff);
    if (image == NULL)
    {
      err = error_OOM;
      goto Failure;
    }
  }

  err = drawable_create(image, &drawable);
  if (err)
    goto Failure;

  viewer->image    = image;
  viewer->drawable = drawable;

  viewer_update(viewer, viewer_UPDATE_ALL);

  set_view_title(viewer);

#ifdef EYE_ZONES
  /* now the viewer block is ready we can create the zones */
  viewer->zones = zones_create(image);
  /* viewer->zones may be NULL on return */
#endif

  /* watch for changes */
  imageobserver_register(image, image_changed_callback);

  /* if we have the focus - inform the image */

  wimp_get_caret_position(&caret);
  if (caret.w == viewer->main_w)
    image_focus(image);

  r = FALSE; /* success */

Exit:

  hourglass_off();

  return r;


Failure:

  image_destroy(image);

  r = TRUE;

  goto Exit;
}

osbool viewer_save(viewer *viewer, const char *file_name)
{
  if (viewer->image->methods.save(&GLOBALS.choices.image,
                                   viewer->image,
                                   file_name))
  {
    return TRUE; /* failure */
  }

  strcpy(viewer->image->file_name, file_name);

  set_view_title(viewer);

  return FALSE; /* success */
}

void viewer_unload(viewer *viewer)
{
  // only if we have focus...
  image_defocus(viewer->image);

  imageobserver_deregister(viewer->image, image_changed_callback);

#ifdef EYE_ZONES
  zones_destroy(viewer->zones);
  viewer->zones = NULL;
#endif

  drawable_destroy(viewer->drawable);
  viewer->drawable = NULL;

  imagecache_destroy(viewer->image);
  viewer->image = NULL;
}

/* ----------------------------------------------------------------------- */

int viewer_query_unload(viewer *viewer)
{
  if (viewer->image->flags & image_FLAG_MODIFIED &&
      viewer_count_clones(viewer) == 1)
  {
    switch (dcs_quit__dcs_query("dcs.modified"))
    {
    case dcs_quit_DISCARD:
      return 1;

    case dcs_quit_CANCEL:
      return 0;

    case dcs_quit_SAVE:
      dialogue__show(save);
      return 0;
    }
  }

  return 1;
}

/* ----------------------------------------------------------------------- */

static int close_all_callback(viewer *viewer, void *arg)
{
  NOT_USED(arg);

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

static void count_edited_callback(image *image, void *arg)
{
  int *nedited = arg;

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

error viewer_choices_updated(const choices_group *g)
{
  NOT_USED(g);

  viewer_update_all(viewer_UPDATE_ALL);

  return error_OK;
}
