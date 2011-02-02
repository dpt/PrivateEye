/* --------------------------------------------------------------------------
 *    Name: display.c
 * Purpose: Displays
 * Version: $Id: display.c,v 1.56 2010-01-06 01:58:53 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/colourtrans.h"
#include "oslib/filer.h"
#include "oslib/fileswitch.h"
#include "oslib/hourglass.h"
#include "oslib/jpeg.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osgbpb.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/types.h"
#include "appengine/app/choices.h"
#include "appengine/dialogues/dcs-quit.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/filer.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/oserror.h"
#include "appengine/wimp/pointer.h"
#include "appengine/vdu/screen.h"
#include "appengine/base/strings.h"
#include "appengine/base/bsearch.h"
#include "appengine/gadgets/hist.h"
#include "appengine/gadgets/metadata.h"

#include "privateeye.h"
#include "actions.h"
#include "choicesdat.h"
#include "clipboard.h"
#include "effects.h"
#include "globals.h"
#include "iconnames.h"          /* generated */
#include "info.h"
#include "keymap.h"
#include "menunames.h"          /* not generated */
#include "rotate.h"
#include "save.h"
#include "scale.h"
#include "tags.h"
#include "to-spr.h"
#include "viewer.h"
#include "zones.h"

#include "actions.h"

#include "display.h"

/* ---------------------------------------------------------------------- */

static event_wimp_handler display__event_redraw_window_request,
                          display__event_close_window_request,
                          display__event_mouse_click,
                          display__event_key_pressed,
                          display__event_menu_selection,
                          display__event_scroll_request,
                          display__event_lose_caret,
                          display__event_gain_caret;

/* ----------------------------------------------------------------------- */

static void display__reg(int reg, viewer_t *viewer)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST,   display__event_redraw_window_request },
    { wimp_CLOSE_WINDOW_REQUEST,    display__event_close_window_request  },
    { wimp_MOUSE_CLICK,             display__event_mouse_click           },
    { wimp_KEY_PRESSED,             display__event_key_pressed           },
    { wimp_SCROLL_REQUEST,          display__event_scroll_request        },
    { wimp_LOSE_CARET,              display__event_lose_caret            },
    { wimp_GAIN_CARET,              display__event_gain_caret            },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            viewer->main_w, event_ANY_ICON,
                            viewer);
}

error display__set_handlers(viewer_t *viewer)
{
  error err;

  display__reg(1, viewer);

  err = help__add_window(viewer->main_w, "display");

  return err;
}

void display__release_handlers(viewer_t *viewer)
{
  help__remove_window(viewer->main_w);

  display__reg(0, viewer);
}

static void display__set_single_handlers(int reg)
{
  /* menu_selection is 'vague' so should only be registered once */
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MENU_SELECTION, display__event_menu_selection },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);
}

/* ----------------------------------------------------------------------- */

error display__init(void)
{
  error err;

  /* dependencies */

  err = help__init();
  if (err)
    return err;

  err = dcs_quit__init();
  if (err)
    return err;

#ifdef EYE_META
  err = metadata__init();
  if (err)
    return err;
#endif

#ifdef EYE_TAGS
  err = tags__init();
  if (err)
    return err;
#endif

  /* handlers */

  display__set_single_handlers(1);

  GLOBALS.display_w = window_create("display");

  err = viewer_save_init();
  if (!err)
    err = viewer_scale_init();
  if (!err)
    err = viewer_info_init();
  if (err)
    return err;

  /* Menu */

  GLOBALS.image_m = menu_create_from_desc(
                                      message0("menu.image"),
                                      dialogue__get_window(viewer_info),
                                      dialogue__get_window(viewer_source_info),
                                      dialogue__get_window(save),
                                      dialogue__get_window(viewer_scale));

  err = help__add_menu(GLOBALS.image_m, "image");
  if (err)
    return err;

  /* Keymap */

  err = viewer_keymap_init();
  if (err)
    return err;

  return error_OK;
}

void display__fin(void)
{
  help__remove_menu(GLOBALS.image_m);

  menu_destroy(GLOBALS.image_m);

  viewer_keymap_fin();
  viewer_info_fin();
  viewer_scale_fin();
  viewer_save_fin();

  display__set_single_handlers(0);

#ifdef EYE_TAGS
  tags__fin();
#endif
#ifdef EYE_META
  metadata__fin();
#endif
  dcs_quit__fin();
  help__fin();
}

/* ----------------------------------------------------------------------- */

static int display__event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_draw *redraw;
  viewer_t  *viewer;
  osbool     more;

  NOT_USED(event_no);

  redraw = &block->redraw;
  viewer = handle;

  if (viewer->background.prepare)
    viewer->background.prepare(viewer);

  for (more = wimp_redraw_window(redraw); more; more = wimp_get_rectangle(redraw))
  {
    int x,y;

    x = redraw->box.x0 - redraw->xscroll;
    y = redraw->box.y1 - redraw->yscroll;

    if (viewer->background.draw)
      viewer->background.draw(redraw, viewer, x, y);

    viewer->drawable->methods.redraw(&GLOBALS.choices.drawable,
                                      redraw,
                                      viewer->drawable,
                                      x + viewer->x,
                                      y + viewer->y);

#ifdef EYE_ZONES
    zones_update(viewer->zones,
                 redraw,
                 viewer->drawable->image,
                 viewer->scale.cur);
#endif
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

typedef void (dragcancel_handler)(void *arg);

static dragcancel_handler *dragcancelfn;
static void               *dragcancelarg;

static void dragcancel__set_handler(int reg, dragcancel_handler *cancelfn, void *arg)
{
  if (reg)
  {
    dragcancelfn  = cancelfn;
    dragcancelarg = arg;
  }
  else
  {
    dragcancelfn  = NULL;
    dragcancelarg = NULL;
  }
}

static int dragcancel_dragging(void)
{
  return dragcancelfn != NULL;
}

static void dragcancel_cancel(void)
{
  if (dragcancelfn)
  {
    wimp_drag_box((wimp_drag *) -1); /* cancel */
    dragcancelfn(dragcancelarg);
    dragcancelfn  = NULL;
    dragcancelarg = NULL;
  }
}

/* ---------------------------------------------------------------------- */

enum { ScrollingScale = 256 };

static event_wimp_handler scrolling__event_null_reason_code;

static void scrolling__set_handlers(int reg, viewer_t *viewer)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, scrolling__event_null_reason_code },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            viewer);

  event_set_interval(0);
}

static int scrolling__event_null_reason_code(wimp_event_no  event_no,
                                             wimp_block    *block,
                                             void          *handle)
{
  viewer_t           *viewer;
  union
  {
    wimp_window_state state;
    wimp_open         open;
  }
  bleh;
  int                 x, y, s;

  NOT_USED(event_no);
  NOT_USED(block);

  viewer = handle;

/* fprintf(stderr,"scrolling\n"); */

  if (viewer->scrolling.count-- == 0)
  {
    scrolling__set_handlers(0, viewer);
    return event_HANDLED;
  }

  bleh.state.w = viewer->main_w;
  wimp_get_window_state(&bleh.state);

  x = viewer->scrolling.pos_x     + viewer->scrolling.dest_x;
  y = viewer->scrolling.pos_y     + viewer->scrolling.dest_y;
  s = viewer->scrolling.pos_scale + viewer->scrolling.dest_scale;
  viewer->scrolling.pos_x     = x;
  viewer->scrolling.pos_y     = y;
  viewer->scrolling.pos_scale = s;

  /* this scale then set order is probably important */

  viewer_scale_set(viewer, s / ScrollingScale, 1);

  /* 'viewer' is possibly stale here now */

  bleh.open.xscroll = x / ScrollingScale;
  bleh.open.yscroll = y / ScrollingScale;

  wimp_open_window(&bleh.open);

  /* now test whether where we've opened is the final resting position, if
   * so cancel any further work */

  return event_HANDLED;
}

static void scrolling_start(viewer_t *viewer, int dx, int dy, int ds,
                            int delta, int steps)
{
  wimp_window_info info;
  int              vw, vh;     /* visible */
  int              cx, cy, cs; /* current */

/* fprintf(stderr, "pan to %d,%d\n", dx, dy); */

  cs = viewer->scale.cur;

  if (ds <= 0) /* desired zoom */
      ds = cs;

  /* where are we? */

  info.w = viewer->main_w;
  wimp_get_window_info_header_only(&info);

  vw = info.visible.x1 - info.visible.x0;
  vh = info.visible.y1 - info.visible.y0;
  cx = info.xscroll;
  cy = info.yscroll;

  /* by how much do we need to move each step? */

  if (delta)
  {
    dx += cx;
    dy += cy;
  }
  else
  {
    /* we position the centre of the view */
    dx -= vw / 2;
    dy += vh / 2;
  }

/* fprintf(stderr, "pan to %d,%d\n", dx, dy); */
/* fprintf(stderr, "curr is %d,%d\n", cx, cy); */

  viewer->scrolling.count      = steps;
  viewer->scrolling.dest_x     = ((dx - cx) * ScrollingScale) / steps;
  viewer->scrolling.dest_y     = ((dy - cy) * ScrollingScale) / steps;
  viewer->scrolling.dest_scale = ((ds - cs) * ScrollingScale) / steps;
  viewer->scrolling.pos_x      = cx * ScrollingScale;
  viewer->scrolling.pos_y      = cy * ScrollingScale;
  viewer->scrolling.pos_scale  = cs * ScrollingScale;

/* fprintf(stderr, "steps of is %f,%f\n", viewer->scrolling.dest_x / 65536.0, viewer->scrolling.dest_y / 65536.0); */

  /* force an immediate scroll */
  /* poll block isn't used so can safely be set to NULL */
  scrolling__event_null_reason_code(wimp_NULL_REASON_CODE, NULL, viewer);

  scrolling__set_handlers(1, viewer);
}

/* ----------------------------------------------------------------------- */

/*
 * step through disc files
 */
static void step(viewer_t *viewer, int direction)
{
  const char         *leaf_name;
  const char         *dir_name;
  int                 context;
  char                file_name[256]; /* Careful Now */
  int                 read_count;
  osgbpb_info_stamped curr[10]; /* space for one struct + buffer space for filename (approx 256 bytes) */

  leaf_name = str_leaf(viewer->drawable->image->file_name);
  dir_name  = str_branch(viewer->drawable->image->file_name);

  context = 0;

  /* scan until we match the current image's leafname */
  do
  {
    EC(xosgbpb_dir_entries_info_stamped(dir_name,
           (osgbpb_info_stamped_list *) curr,
                                        1, /* count */
                                        context,
                                        sizeof(curr), /* size */
                                        leaf_name,
                                       &read_count,
                                       &context));
  }
  while (read_count == 0 && context > -1);

  if (context < 0)
    return; /* couldn't find the original file */

  do
  {
    do
    {
      /* if we're moving backwards then skip back by *two* entries. */
      /* note the assumption that the context values returned by OS_GBPB are
       * indices */
      if (direction < 0)
        context -= 2;

      EC(xosgbpb_dir_entries_info_stamped(dir_name,
             (osgbpb_info_stamped_list *) curr,
                                          1, /* count */
                                          context,
                                          sizeof(curr), /* size */
                                          "*",
                                         &read_count,
                                         &context));
    }
    while (read_count == 0 && context > -1);

    if (context < 0)
      return; /* couldn't find the file */
  }
  while (!image_is_loadable(curr[0].file_type));

  /* now we've got a file, and it's loadable */

  if (!viewer_query_unload(viewer))
    return;

  viewer_unload(viewer);

  str_cpy(file_name, dir_name);
  strcat(file_name, ".");
  strcat(file_name, curr[0].name);

  if (viewer_load(viewer, file_name, curr[0].load_addr, curr[0].exec_addr))
  {
    viewer_destroy(viewer);
    /* FIXME: break out and return */
  }
  else
  {
    viewer_open(viewer);
  }
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler grab__event_user_drag_box,
                          grab__event_pollword_non_zero;

static int grabx, graby;
static int grabminx, grabminy;

static void grab__set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX,     grab__event_user_drag_box     },
    { wimp_POLLWORD_NON_ZERO, grab__event_pollword_non_zero },
  };

/*  printf("grab__set_handlers %d\n", reg); */

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);
}

static void grab__end(void *arg)
{
  NOT_USED(arg);

/*  printf("grab__end\n"); */

  restore_pointer_shape();

  grab__set_handlers(0);
}

static int grab__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer    *pointer;
  viewer_t        *viewer;
  wimp_window_info info;
  int              x,y;
  wimp_drag        drag;
  int              xvis,yvis;
  int              xscr,yscr;
  int              xext,yext;

/*  printf("grab__event_mouse_click\n"); */

  NOT_USED(event_no);

  pointer = &block->pointer;
  viewer  = handle;

  info.w = viewer->main_w;
  wimp_get_window_info(&info);

  grabx = pointer->pos.x;
  graby = pointer->pos.y;
  grabminx = info.xscroll;
  grabminy = info.yscroll;

  x = pointer->pos.x;
  y = pointer->pos.y;

  drag.type = wimp_DRAG_ASM_FIXED;

  drag.initial.x0 = x;
  drag.initial.y0 = y;
  drag.initial.x1 = x;
  drag.initial.y1 = y;

  /* parent bounds */
  /* position a leeway-sized box around the pointer position */

  xvis = info.visible.x1 - info.visible.x0;
  yvis = info.visible.y1 - info.visible.y0;

  xscr = info.xscroll - info.extent.x0;
  yscr = info.yscroll - info.extent.y0;

  xext = info.extent.x1 - info.extent.x0;
  yext = info.extent.y1 - info.extent.y0;

  drag.bbox.x0 = x - (- xvis - xscr + xext);
  drag.bbox.y0 = y - (       - yscr + yext);
  drag.bbox.x1 = x + (         xscr       );
  drag.bbox.y1 = y + (- yvis + yscr       );

  drag.handle = event_get_pollword();
  _swi(0x4D942 /* AppEngine_WindowOp */, _IN(0)|_OUTR(0,2),
       8, &drag.draw, &drag.undraw, &drag.redraw);

  wimp_drag_box(&drag);

  GLOBALS.dragging_viewer = viewer;

  set_pointer_shape("ptr_grab", 4, 4);

  grab__set_handlers(1);

  dragcancel__set_handler(1, grab__end, NULL);

  return event_HANDLED;
}

static int grab__event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

/*  printf("grab__event_user_drag_box\n"); */

  dragcancel__set_handler(0, grab__end, NULL);

  grab__end(NULL);

  return event_HANDLED;
}

static int grab__event_pollword_non_zero(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer        pointer;
  union
  {
    wimp_window_state state;
    wimp_open         open;
  }
  bleh;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  event_zero_pollword();

  wimp_get_pointer_info(&pointer);

  bleh.state.w = GLOBALS.dragging_viewer->main_w;
  wimp_get_window_state(&bleh.state);

  bleh.open.xscroll = grabminx + grabx - pointer.pos.x;
  bleh.open.yscroll = grabminy + graby - pointer.pos.y;

  wimp_open_window(&bleh.open);

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static int display__event_close_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_close  *close;
  viewer_t    *viewer;
  wimp_pointer pointer;

  NOT_USED(event_no);

  close  = &block->close;
  viewer = handle;

  wimp_get_pointer_info(&pointer);

  /* Note that we might be entered if another part of the program has
   * faked a close event, in which case the pointer may or may not have
   * buttons held.
   */

  if (pointer.buttons & wimp_CLICK_ADJUST)
  {
    filer_open_dir(viewer->drawable->image->file_name);

    if (inkey(INKEY_SHIFT))
      return event_HANDLED;
  }

  if (viewer_query_unload(viewer))
  {
    viewer_unload(viewer);
    viewer_destroy(viewer);
  }

  return event_HANDLED;
}

static void pan_to_point(wimp_pointer *pointer, viewer_t *viewer)
{
  wimp_window_info info;
  int              wax, way; /* work area x,y */

  info.w = GLOBALS.current_display_w;
  wimp_get_window_info_header_only(&info);

  wax = pointer->pos.x + (info.xscroll - info.visible.x0);
  way = pointer->pos.y + (info.yscroll - info.visible.y1);

  scrolling_start(viewer, wax, way, 0, 0, GLOBALS.choices.viewer.steps);
}

static void zoom_to_point(wimp_pointer *pointer, viewer_t *viewer)
{
  wimp_window_info info;
  int              wax, way; /* work area x,y */
  int              visible_w, visible_h;
  int              old_scale, new_scale;

  info.w = GLOBALS.current_display_w;
  wimp_get_window_info_header_only(&info);

  wax = pointer->pos.x + (info.xscroll - info.visible.x0);
  way = pointer->pos.y + (info.yscroll - info.visible.y1);

  visible_w = info.visible.x1 - info.visible.x0;
  visible_h = info.visible.y1 - info.visible.y0;

  old_scale = viewer->scale.cur;
  if (pointer->buttons & wimp_CLICK_SELECT)
    new_scale = old_scale * 2;
  else
    new_scale = old_scale / 2;
#if 0
  viewer_scale_set(GLOBALS.current_display_w, new_scale, 0);

  info.xscroll = (wax * new_scale / old_scale) - visible_w / 2;
  info.yscroll = (way * new_scale / old_scale) + visible_h / 2;
  wimp_open_window((wimp_open *) &info);

  window_redraw((int) GLOBALS.current_display_w);
#else
  wax = (wax * new_scale / old_scale);
  way = (way * new_scale / old_scale);

  scrolling_start(viewer, wax, way, new_scale, 0,
                  GLOBALS.choices.viewer.steps);
#endif
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler embed__event_user_drag_box;

static void embed__set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX, embed__event_user_drag_box },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);
}

static void embed__end(void *arg)
{
  NOT_USED(arg);

  restore_pointer_shape();

  embed__set_handlers(0);
}

static int embed__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer    *pointer;
  viewer_t        *viewer;
  wimp_window_info info;
  wimp_drag        drag;

  NOT_USED(event_no);
  NOT_USED(block);

  pointer = &block->pointer;
  viewer  = handle;

  info.w = viewer->main_w;
  wimp_get_window_info_header_only(&info);

  /* dragging to embed the window */

  drag.w = pointer->w;
  drag.type = wimp_DRAG_USER_POINT;

  read_drag_box_for_screen(&drag.bbox);

  wimp_drag_box(&drag);

  set_pointer_shape("ptr_embed", 0, 0);

  embed__set_handlers(1);

  dragcancel__set_handler(1, embed__end, NULL);

  return event_HANDLED;
}

static int embed__event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer     pointer;
  wimp_w           parent;
  wimp_window_info info;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  dragcancel__set_handler(0, embed__end, NULL);

  embed__end(NULL);

  wimp_get_pointer_info(&pointer);

  if (pointer.w == GLOBALS.current_display_w)
    return event_HANDLED; /* can't embed in self */

  parent = (pointer.w == wimp_ICON_BAR) ? wimp_TOP : pointer.w;

  info.w = GLOBALS.current_display_w;
  wimp_get_window_info_header_only(&info);

  wimp_open_window_nested((wimp_open *) &info,
                          parent,
                          wimp_CHILD_LINKS_PARENT_WORK_AREA);

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler zoombox__event_user_drag_box;

static void zoombox__set_handlers(int reg, viewer_t *viewer)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX, zoombox__event_user_drag_box },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            viewer);
}

static void zoombox__end(void *arg)
{
  /* Turn off autoscrolling */
  if (GLOBALS.wimp_version >= 399) // should 399 be wimp_VERSION_RO40?
    wimp_auto_scroll(0, NULL);

  restore_pointer_shape();

  zoombox__set_handlers(0, arg);
}

static int zoombox__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer    *pointer;
  viewer_t        *viewer;
  wimp_window_info info;
  wimp_drag        drag;
#if 0
  int              x, y;
#endif

  NOT_USED(event_no);

  pointer = &block->pointer;
  viewer  = handle;

  info.w = viewer->main_w;
  wimp_get_window_info_header_only(&info);

  /* dragbox zoom */

  drag.w = pointer->w;
  drag.type = wimp_DRAG_USER_RUBBER;

  drag.initial.x0 = pointer->pos.x;
  drag.initial.y0 = pointer->pos.y;
  drag.initial.x1 = pointer->pos.x;
  drag.initial.y1 = pointer->pos.y;

#if 0
  x = info.visible.x0 - info.xscroll;
  y = info.visible.y1 - info.yscroll;

  drag.bbox.x0 = GLOBALS.scaling_drag_bbox.x0 = info.extent.x0 + x;
  drag.bbox.y0 = GLOBALS.scaling_drag_bbox.y0 = info.extent.y0 + y;
  drag.bbox.x1 = GLOBALS.scaling_drag_bbox.x1 = info.extent.x1 + x;
  drag.bbox.y1 = GLOBALS.scaling_drag_bbox.y1 = info.extent.y1 + y;
#else
  drag.bbox.x0 = GLOBALS.scaling_drag_bbox.x0 = -0x3fffffff;
  drag.bbox.y0 = GLOBALS.scaling_drag_bbox.y0 = -0x3fffffff;
  drag.bbox.x1 = GLOBALS.scaling_drag_bbox.x1 =  0x3fffffff;
  drag.bbox.y1 = GLOBALS.scaling_drag_bbox.y1 =  0x3fffffff;
#endif

  if (GLOBALS.wimp_version >= 399)
  {
    wimp_auto_scroll_info info;

    info.w = pointer->w;
    info.pause_zone_sizes.x0 =
    info.pause_zone_sizes.y0 =
    info.pause_zone_sizes.x1 =
    info.pause_zone_sizes.y1 = 16;
    info.pause_duration = 0; /* don't pause */
    info.state_change = wimp_AUTO_SCROLL_NO_HANDLER; /* using own pointer */
    /* info.handle = 0; N/A */

    wimp_auto_scroll(wimp_AUTO_SCROLL_ENABLE_HORIZONTAL |
                     wimp_AUTO_SCROLL_ENABLE_VERTICAL,
                    &info);

    wimp_drag_box_with_flags(&drag,
                              wimp_DRAG_BOX_KEEP_IN_LINE |
                              wimp_DRAG_BOX_CLIP);
  }
  else
  {
    wimp_drag_box(&drag);
  }

  set_pointer_shape("ptr_mag", 10, 9);

  zoombox__set_handlers(1, viewer);

  dragcancel__set_handler(1, zoombox__end, viewer);

  return event_HANDLED;
}

static int zoombox__event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_dragged    *dragged;
  viewer_t        *viewer;
  wimp_window_info info;
  int              extent_w, extent_h;
  int              x0, x1, y0, y1;
  int              w, h; /* selection width, height */
  int              old_scale, new_scale;
  int              temp;

  NOT_USED(event_no);

  dragged = &block->dragged;
  viewer  = handle;

  dragcancel__set_handler(0, zoombox__end, viewer);

  zoombox__end(viewer);

  /* Read the window coordinates */
  info.w = GLOBALS.current_display_w;
  wimp_get_window_info_header_only(&info);

  extent_w = info.extent.x1 - info.extent.x0;
  extent_h = info.extent.y1 - info.extent.y0;

  x0 = dragged->final.x0 - info.visible.x0 + info.xscroll;
  x1 = dragged->final.x1 - info.visible.x0 + info.xscroll;
  y0 = dragged->final.y0 - info.visible.y1 + info.yscroll;
  y1 = dragged->final.y1 - info.visible.y1 + info.yscroll;

  if (x0 > x1) { temp = x1; x1 = x0; x0 = temp; }
  if (y0 > y1) { temp = y1; y1 = y0; y0 = temp; }

  w = x1 - x0;
  h = y1 - y0;

  /*
  fprintf(stderr, "extent_w=%d extent_h=%d\n",extent_w,extent_h);
  fprintf(stderr, "x0=%d x1=%d y0=%d y1=%d w=%d h=%d\n",x0,x1,y0,y1,w,h);
  */

  old_scale = viewer->scale.cur;

  if (w >= 8 && h >= 8)
  {
    /* Scale to the largest dimension */
    if (w >= h)
      new_scale = extent_w * SCALE_100PC / w;
    else
      new_scale = extent_h * SCALE_100PC / h;

    if (old_scale != new_scale)
    {
      viewer_scale_set(viewer, new_scale, 0);

      /* Set middle point of user's selection (ish) */
      info.xscroll = x0 * new_scale / old_scale;
      info.yscroll = y1 * new_scale / old_scale;

      wimp_open_window((wimp_open *) &info);

      window_redraw(GLOBALS.current_display_w);
    }
  }

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static void disable(wimp_menu *m, int entry, int available)
{
  menu_set_icon_flags(m,
                      entry,
                      available ? 0 : wimp_ICON_SHADED,
                      wimp_ICON_SHADED);
}

/*

typedef struct menutest_entry
{
  int (*test)(const image *);
}
menutest_entry;

typedef struct menutest_menu
{
  int             menu;
  menutest_entry *entries;
  int             nentries;
}
menutest_menu;

static
menutest_entry misc_entries[] = { NULL,
                                  NULL,
                                  metadata__available,
                                  hist__available,
                                  NULL };

static
menutest_entry edit_entries[] = { effects__available,
                                  rotate__available,
                                  to_spr__available,
                                  NULL,
                                  NULL };

static
menutest_menu menus[] = { { IMAGE_MISC, misc_entries, NELEMS(misc_entries) },
                          { IMAGE_EDIT, edit_entries, NELEMS(edit_entries) } };

for (i = 0; i < NELEMS(menus); i++)
{
  menutest_menu *mtm;

  mtm = &menus[i];

  m = entries[mtm->menu].sub_menu;

  for (j = 0; j < mtm->nentries; j++)
  {
    if (mtm->entries[j])
      disable(m, j, mtm->entries[j](image));
  }
}

*/

static void display__menu_update(void)
{
  viewer_t        *viewer;
  image_t         *image;
  wimp_menu_entry *entries;
  wimp_menu       *m;

  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  entries = GLOBALS.image_m->entries;

  /* Misc menu */

  m = entries[IMAGE_FILE].sub_menu;

  /* Shade the "Histogram" entry if we don't have that method */
  disable(m, FILE_HIST, hist__available(image));

#ifdef EYE_META
  /* Shade the "Metadata" entry if we don't have that method */
  disable(m, FILE_METADATA, metadata__available(image));
#endif

  /* Edit menu */

  m = entries[IMAGE_EDIT].sub_menu;

  /* Shade the "Effects" entry if effects module says "no" */
  disable(m, EDIT_EFFECTS, effects__available(image));

  /* Shade the "Rotate" entry if rotate module says "no" */
  disable(m, EDIT_ROTATE,  rotate__available(image));

  /* Shade the "Convert to Sprite" entry if to_spr module says "no" */
  disable(m, EDIT_CONVERT_TO_SPRITE, to_spr__available(image));

  /* Shade the "Release clipboard" entry if we don't own the clipboard */
  disable(m, EDIT_RELEASE, clipboard_own());
}


static int display__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
#define KeyNone         0
#define KeyShift        1
#define KeyCtrl         2
#define KeyShiftCtrl    3
#define KeyAlt          4
#define KeyShiftAlt     5
#define KeyCtrlAlt      6
#define KeyShiftCtrlAlt 7

  wimp_pointer *pointer;
  viewer_t     *viewer;
  osbool        shift;
  osbool        ctrl;
  osbool        alt;
  unsigned int  keys;

  pointer = &block->pointer;
  viewer  = handle;

  GLOBALS.current_display_w = viewer->main_w;

  shift = inkey(INKEY_SHIFT);
  ctrl  = inkey(INKEY_CTRL);
  alt   = inkey(INKEY_ALT);

  keys = (shift << 0) | (ctrl << 1) | (alt << 2);

#ifndef NDEBUG
  fprintf(stderr, "click: display: type = %d, shift,ctrl,alt = %d,%d,%d\n",
          pointer->buttons, shift, ctrl, alt);
#endif

  if (pointer->buttons & wimp_CLICK_MENU)
  {
    display__menu_update();

    menu_open(GLOBALS.image_m, pointer->pos.x - 64, pointer->pos.y);
  }
  else if (pointer->buttons & wimp_CLICK_SELECT)
  {
    switch (keys)
    {
      case KeyShift: zoom_to_point(pointer, viewer); break;
      case KeyCtrl: pan_to_point(pointer, viewer); break;
      default: break;
    }
  }
  else if (pointer->buttons & wimp_CLICK_ADJUST)
  {
    switch (keys)
    {
      case KeyShift: zoom_to_point(pointer, viewer); break;
      default: break;
    }
  }
  else if (pointer->buttons & wimp_DRAG_SELECT)
  {
    switch (keys)
    {
      case KeyNone: grab__event_mouse_click(event_no, block, handle); break;
      case KeyShiftCtrl: embed__event_mouse_click(event_no, block, handle); break;
      default: break;
    }
  }
  else if (pointer->buttons & wimp_DRAG_ADJUST)
  {
    switch (keys)
    {
      case KeyNone: zoombox__event_mouse_click(event_no, block, handle); break;
      default: break;
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static void action_zoom(viewer_t *viewer, int op)
{
  int old_scale = viewer->scale.cur;
  int new_scale;

  switch (op)
  {
  case ZoomIn:     new_scale = old_scale * 2; break;
  case ZoomOut:    new_scale = old_scale / 2; break;
  case ZoomReset:  new_scale = 100; break;
  default:
  case ZoomToggle: new_scale = viewer->scale.prev; break;
  }

  viewer_scale_set(viewer, new_scale, 1);
}

static void action_step(viewer_t *viewer, int op)
{
  step(viewer, (op == StepForwards) ? 1 : -1);
}

static void action_panrand(viewer_t *viewer, int op)
{
  wimp_window_info info;
  int              x, y;

  NOT_USED(op);

  info.w = viewer->main_w;
  wimp_get_window_info_header_only(&info);

  /* pick a coordinate to *centre* on */

  x = info.extent.x0 + rand() % (info.extent.x1 - info.extent.x0);
  y = info.extent.y0 + rand() % (info.extent.y1 - info.extent.y0);

  scrolling_start(viewer, x, y, 0, 0, GLOBALS.choices.viewer.steps);
}

static void action_pan(viewer_t *viewer, int op)
{
  if (viewer->scrolling.count > 0)
  {
    /* already scrolling - force an update */
    scrolling__event_null_reason_code(wimp_NULL_REASON_CODE, NULL, viewer);
  }
  else
  {
    int panx, pany;
    int x, y;

    panx = GLOBALS.choices.viewer.scroll_x;
    pany = GLOBALS.choices.viewer.scroll_y;

    x = 0;
    y = 0;

    if      (op == PanUp)    y =  pany;
    else if (op == PanDown)  y = -pany;
    else if (op == PanLeft)  x = -panx;
    else if (op == PanRight) x =  panx;

    /* x and y are deltas */
    scrolling_start(viewer, x, y, 0, 1, GLOBALS.choices.viewer.steps);
  }
}

static void action_fliprot(viewer_t *viewer, int op)
{
  int r;
  int f;

  switch (op)
  {
    default:
    case HorzFlip:    r = 0; f = 1; break;
    case RotateLeft:  r = 1; f = 0; break;
    case VertFlip:    r = 2; f = 1; break;
    case RotateRight: r = 3; f = 0; break;
  }

  rotate(viewer->drawable->image, r * 90 * 65536, f);
}

static void action_kill(viewer_t *viewer)
{
  os_error         *e;
  wimp_t            act;
  char             *file_name;
  const char       *branch;
  const char       *leaf;
  fileraction_flags flags;

  e = EC(xwimp_start_task("Filer_Action", &act));
  if (e)
    goto failure;

  file_name = viewer->drawable->image->file_name;
  branch    = str_branch(file_name);
  leaf      = str_leaf(file_name);

  /* My intention here was to read the CMOS RAM and pass the same flags
   * currently in effect in the Filer into the Filer_Action.
   * Unfortunately since the Configure changes introduced in RISC OS 3.8,
   * you can't do that. The Filer no longer maintains its CMOS RAM byte.
   */

  flags = 0;

  e = EC(xfileraction_send_selected_directory(act, branch));
  if (!e)
    e = EC(xfileraction_send_selected_file(act, leaf));
  if (!e)
    e = EC(xfileractionsendstartoperation_delete(act, flags));
  if (e)
    goto failure;

  /* FIXME: Automatically step to next image? */

  return;


failure:

  oserror__report_block(e);
}

static void action(viewer_t *viewer, int op)
{
  switch (op)
  {
  case ZoomIn:
  case ZoomOut:
  case ZoomReset:
  case ZoomToggle:
    action_zoom(viewer, op);
    break;

  case StepForwards:
  case StepBackwards:
    action_step(viewer, op);
    break;

  case PanRandom:
    action_panrand(viewer, op);
    break;

  case PanUp:
  case PanDown:
  case PanLeft:
  case PanRight:
    action_pan(viewer, op);
    break;

  case HorzFlip:
  case RotateLeft:
  case RotateRight:
  case VertFlip:
    action_fliprot(viewer, op);
    break;

  case Close:
    error__report(action_close_window(viewer->main_w));
    break;

  case Save:
    dialogue__show(save);
    break;

  case Info:
    dialogue__show(viewer_info);
    break;

  case SourceInfo:
    dialogue__show(viewer_source_info);
    break;

  case Hist:
    hist__open(viewer->drawable->image, GLOBALS.choices.hist.bars);
    break;

  case Rotate:
    rotate__open(viewer->drawable->image);
    break;

  case Scale:
    dialogue__show(viewer_scale);
    break;

#ifdef EYE_TAGS
  case Tags:
    error__report(tags__open(viewer->drawable->image));
    break;
#endif

  case Copy:
    clipboard_claim(viewer->main_w);
    break;

  case Help:
    error__report(action_help());
    break;

  case Kill:
    action_kill(viewer);
    break;

  case NewView:
    viewer_clone(viewer);
    break;

  case Release:
    clipboard_release();
    break;

  case Effects:
    effects__open(viewer->drawable->image);
    break;

  case MetaData:
#ifdef EYE_META
    metadata__open(viewer->drawable->image,
                   GLOBALS.choices.metadata.bgcolour,
                   GLOBALS.choices.metadata.wrapwidth,
                   GLOBALS.choices.metadata.line_height);
#endif
    break;

  case ConvToSpr:
    to_spr(viewer->drawable->image);
    break;
  }
}

static int display__event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_key *key;
  viewer_t *viewer;
  int       op;
  osbool    close_menus = FALSE; /* set when a view change will invalidate any displayed data: menus need closing, where open */

  NOT_USED(event_no);

  key    = &block->key;
  viewer = handle;

  /* Special case Escape when dragging */
  if (key->c == wimp_KEY_ESCAPE)
  {
/*    printf("Escape pressed\n"); */
    if (dragcancel_dragging())
    {
/*      printf("calling dragcancel\n"); */
      dragcancel_cancel();
      return event_HANDLED;
    }
  }

  op = viewer_keymap_op(viewer_keymap_SECTION_VIEWER, key->c);
  if (op < 0) /* unknown op */
  {
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  action(viewer, op);

  if (op == StepForwards || op == StepBackwards || op == Close)
    close_menus = TRUE;

  if (close_menus && menu_last() == GLOBALS.image_m)
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);

  return event_HANDLED;
}

static int display__event_menu_selection(wimp_event_no event_no, wimp_block *block, void *handle)
{
#define PACK(a,b) (((a) << 8) | (b))

  static const struct
  {
    unsigned int items;
    int          op;
  }
  map[] =
  {
#ifdef EYE_META
    { PACK(IMAGE_FILE, FILE_METADATA),            MetaData  },
#endif
    { PACK(IMAGE_FILE, FILE_HIST),                Hist      },
#ifdef EYE_TAGS
    { PACK(IMAGE_FILE, FILE_TAGS),                Tags      },
#endif
    { PACK(IMAGE_FILE, FILE_NEWVIEW),             NewView   },
    { PACK(IMAGE_EDIT, EDIT_EFFECTS),             Effects   },
    { PACK(IMAGE_EDIT, EDIT_ROTATE),              Rotate    },
    { PACK(IMAGE_EDIT, EDIT_CONVERT_TO_SPRITE),   ConvToSpr },
    { PACK(IMAGE_EDIT, EDIT_CLAIM),               Copy      },
    { PACK(IMAGE_EDIT, EDIT_RELEASE),             Release   },
  };

  const size_t stride = sizeof(map[0]);
  const size_t nelems = sizeof(map) / stride;

  wimp_selection *selection;
  wimp_menu      *last;
  wimp_pointer    p;
  viewer_t       *viewer;
  unsigned int    item;
  int             i;

  NOT_USED(event_no);
  NOT_USED(handle);

  selection = &block->selection;

  last = menu_last();
  if (last != GLOBALS.image_m)
    return event_NOT_HANDLED;

  item = PACK(selection->items[0], selection->items[1]);

  i = bsearch_uint(&map[0].items, nelems, stride, item);
  if (i >= 0)
  {
    viewer = viewer_find(GLOBALS.current_display_w);
    if (viewer == NULL)
      return event_HANDLED;

    action(viewer, map[i].op);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    display__menu_update();
    menu_reopen();
  }

  return event_HANDLED;

#undef PACK
}

static int display__event_scroll_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_scroll *scroll;
  viewer_t    *viewer;
  int          d;

  NOT_USED(event_no);

  scroll = &block->scroll;
  viewer = handle;

  switch (scroll->xmin)
  {
  default:
    d = 0;
    break;
  case wimp_SCROLL_PAGE_LEFT:
    d = -(scroll->visible.x1 - scroll->visible.x0);
    break;
  case wimp_SCROLL_PAGE_RIGHT:
    d = scroll->visible.x1 - scroll->visible.x0;
    break;
  case wimp_SCROLL_COLUMN_LEFT:
    d = -GLOBALS.choices.viewer.scroll_x;
    break;
  case wimp_SCROLL_COLUMN_RIGHT:
    d = GLOBALS.choices.viewer.scroll_x;
    break;
  }
  scroll->xscroll += d;

  switch (scroll->ymin)
  {
  default:
    d = 0;
    break;
  case wimp_SCROLL_PAGE_UP:
    d = scroll->visible.y1 - scroll->visible.y0;
    break;
  case wimp_SCROLL_PAGE_DOWN:
    d = -(scroll->visible.y1 - scroll->visible.y0);
    break;
  case wimp_SCROLL_LINE_UP:
    d = GLOBALS.choices.viewer.scroll_y;
    break;
  case wimp_SCROLL_LINE_DOWN:
    d = -GLOBALS.choices.viewer.scroll_y;
    break;
  }
  scroll->yscroll += d;

  wimp_open_window((wimp_open *) scroll);

  return event_HANDLED;
}

static int display__event_lose_caret(wimp_event_no event_no, wimp_block *block, void *handle)
{
  viewer_t *viewer;

  NOT_USED(event_no);
  NOT_USED(block);

  viewer = handle;

  image_defocus(viewer->drawable->image);

  return event_HANDLED;
}


static int display__event_gain_caret(wimp_event_no event_no, wimp_block *block, void *handle)
{
  viewer_t *viewer;

  NOT_USED(event_no);
  NOT_USED(block);

  viewer = handle;

  image_focus(viewer->drawable->image);

  return event_HANDLED;
}
