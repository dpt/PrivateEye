/* --------------------------------------------------------------------------
 *    Name: display.c
 * Purpose: Displays
 * ----------------------------------------------------------------------- */

// TODO
// - split out the sub-modules like dragcancel, zoombox, to their own source
// files
// - cut down the #includes

#include "kernel.h"
#include "swis.h"

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/colourtrans.h"
#include "oslib/filer.h"
#include "oslib/fileswitch.h"
#include "oslib/hourglass.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osgbpb.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/types.h"
#include "appengine/app/choices.h"
#include "appengine/app/wire.h"
#include "appengine/base/bsearch.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"
#include "appengine/dialogues/dcs-quit.h"
#include "appengine/gadgets/hist.h"
#include "appengine/gadgets/metadata.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/filer.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/pointer.h"

#include "actions.h"
#include "choicesdat.h"
#include "clipboard.h"
#include "effects.h"
#include "globals.h"
#include "iconnames.h"          /* generated */
#include "info.h"
#include "keymap.h"
#include "menunames.h"          /* not generated */
#include "privateeye.h"
#include "rotate.h"
#include "save.h"
#include "scale.h"
#include "tags.h"
#include "to-spr.h"
#include "viewer.h"
#include "zones.h"

#include "display.h"

/* ---------------------------------------------------------------------- */

static struct
{
  viewer_keymap_id keymap_id;
}
LOCALS;

/* ---------------------------------------------------------------------- */

/* key defns */
enum
{
  ConvToSpr,
  Copy,
  Effects,
  Hist,
  HorzFlip,
  Info,
  Kill,
  MetaData,
  NewView,
  PanDown,
  PanLeft,
  PanRandom,
  PanRight,
  PanUp,
  Release,
  Rotate,
  RotateLeft,
  RotateRight,
  Save,
  Scale,
  SourceInfo,
  StepBackwards,
  StepForwards,
  Tags,
  VertFlip,
  ZoomIn,
  ZoomOut,
  ZoomReset,
  ZoomToggle,
};

/* ---------------------------------------------------------------------- */

static result_t declare_keymap(void)
{
  /* Keep these sorted by name */
  static const keymap_name_to_action keys[] =
  {
    { "ConvToSpr",        ConvToSpr               },
    { "Copy",             Copy                    },
    { "Effects",          Effects                 },
    { "Hist",             Hist                    },
    { "HorzFlip",         HorzFlip                },
    { "Info",             Info                    },
    { "Kill",             Kill                    },
#ifdef EYE_META
    { "MetaData",         MetaData                },
#endif
    { "NewView",          NewView                 },
    { "PanDown",          PanDown                 },
    { "PanLeft",          PanLeft                 },
    { "PanRandom",        PanRandom               },
    { "PanRight",         PanRight                },
    { "PanUp",            PanUp                   },
    { "Rotate",           Rotate                  },
    { "RotateLeft",       RotateLeft              },
    { "RotateRight",      RotateRight             },
    { "Save",             Save                    },
    { "Scale",            Scale                   },
    { "SourceInfo",       SourceInfo              },
    { "StepBackwards",    StepBackwards           },
    { "StepForwards",     StepForwards            },
#ifdef EYE_TAGS
    { "Tags",             Tags                    },
#endif
    { "VertFlip",         VertFlip                },
    { "ZoomIn",           ZoomIn                  },
    { "ZoomOut",          ZoomOut                 },
    { "ZoomReset",        ZoomReset               },
    { "ZoomToggle",       ZoomToggle              },
  };

  return viewer_keymap_add("Viewer",
                           keys,
                           NELEMS(keys),
                           &LOCALS.keymap_id);
}

static result_t display_substrate_callback(const wire_message_t *message,
                                           void                 *opaque)
{
  NOT_USED(opaque);

  switch (message->event)
  {
    case wire_event_DECLARE_KEYMAP:
      return declare_keymap();
  }

  return result_OK;
}

result_t display_substrate_init(void)
{
  result_t err;

  err = wire_register(0, display_substrate_callback, NULL, NULL);
  if (err)
    return err;

  return result_OK;
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler display_event_redraw_window_request,
                          display_event_close_window_request,
                          display_event_mouse_click,
                          display_event_key_pressed,
                          display_event_menu_selection,
                          display_event_scroll_request,
                          display_event_lose_caret,
                          display_event_gain_caret;

/* ----------------------------------------------------------------------- */

static void display_reg(int reg, viewer_t *viewer)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, display_event_redraw_window_request },
    { wimp_CLOSE_WINDOW_REQUEST,  display_event_close_window_request  },
    { wimp_MOUSE_CLICK,           display_event_mouse_click           },
    { wimp_KEY_PRESSED,           display_event_key_pressed           },
    { wimp_SCROLL_REQUEST,        display_event_scroll_request        },
    { wimp_LOSE_CARET,            display_event_lose_caret            },
    { wimp_GAIN_CARET,            display_event_gain_caret            },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            viewer->main_w,
                            event_ANY_ICON,
                            viewer);
}

result_t display_set_handlers(viewer_t *viewer)
{
  result_t err;

  display_reg(1, viewer);

  err = help_add_window(viewer->main_w, "display");

  return err;
}

void display_release_handlers(viewer_t *viewer)
{
  help_remove_window(viewer->main_w);

  display_reg(0, viewer);
}

static void display_set_single_handlers(int reg)
{
  /* menu_selection is 'vague' so should only be registered once */
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MENU_SELECTION, display_event_menu_selection },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            NULL);
}

/* ----------------------------------------------------------------------- */

result_t display_init(void)
{
  typedef result_t (*initfn)(void);

  static const initfn initfns[] =
  {
    help_init,
    dcs_quit_init,
    hist_init,
#ifdef EYE_META
    metadata_init,
#endif
#ifdef EYE_TAGS
    tags_init,
#endif
  };

  result_t err;
  int      i;

  /* initialise dependencies */

  for (i = 0; i < NELEMS(initfns); i++)
  {
    err = initfns[i]();
    if (err)
      return err;
  }

  /* handlers */

  display_set_single_handlers(1);

  GLOBALS.display_w = window_create("display");

  err = viewer_savedlg_init();
  if (!err)
    err = viewer_scaledlg_init();
  if (!err)
    err = viewer_infodlg_init();
  if (err)
    return err;

  /* Menu */

  GLOBALS.image_m = menu_create_from_desc(
                                      message0("menu.image"),
                                      dialogue_get_window(viewer_infodlg),
                                      dialogue_get_window(viewer_srcinfodlg),
                                      dialogue_get_window(viewer_savedlg),
                                      dialogue_get_window(viewer_scaledlg));

  err = help_add_menu(GLOBALS.image_m, "image");
  if (err)
    return err;

  /* Keymap */

  err = viewer_keymap_init();
  if (err)
    return err;

  return result_OK;
}

void display_fin(void)
{
  typedef void (*finfn)(void);

  static const finfn finfns[] =
  {
#ifdef EYE_TAGS
    tags_fin,
#endif
#ifdef EYE_META
    metadata_fin,
#endif
    hist_fin,
    dcs_quit_fin,
    help_fin,
  };

  int i;

  help_remove_menu(GLOBALS.image_m);

  menu_destroy(GLOBALS.image_m);

  viewer_keymap_fin();
  viewer_infodlg_fin();
  viewer_scaledlg_fin();
  viewer_savedlg_fin();

  display_set_single_handlers(0);

  for (i = 0; i < NELEMS(finfns); i++)
    finfns[i]();
}

/* ----------------------------------------------------------------------- */

static int display_event_redraw_window_request(wimp_event_no event_no,
                                               wimp_block   *block,
                                               void         *handle)
{
  wimp_draw *redraw;
  viewer_t  *viewer;
  osbool     more;

  NOT_USED(event_no);

  redraw = &block->redraw;
  viewer = handle;

  if (viewer->background.prepare)
    viewer->background.prepare(viewer);

  for (more = wimp_redraw_window(redraw);
       more;
       more = wimp_get_rectangle(redraw))
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

typedef void (dragcancel_handler)(void *opaque);

static dragcancel_handler *dragcancelhandler;
static void               *dragcancelopaque;

static void dragcancel_set_handler(int                 reg,
                                   dragcancel_handler *handler,
                                   void               *opaque)
{
  if (reg)
  {
    dragcancelhandler = handler;
    dragcancelopaque  = opaque;
  }
  else
  {
    dragcancelhandler = NULL;
    dragcancelopaque  = NULL;
  }
}

static int dragcancel_dragging(void)
{
  return dragcancelhandler != NULL;
}

static void dragcancel_cancel(void)
{
  if (dragcancelhandler)
  {
    wimp_drag_box((wimp_drag *) -1); /* cancel */
    dragcancelhandler(dragcancelopaque);
    dragcancelhandler = NULL;
    dragcancelopaque  = NULL;
  }
}

/* ---------------------------------------------------------------------- */

enum { ScrollingScale = 256 };

static event_wimp_handler scrolling_event_null_reason_code;

static void scrolling_set_handlers(int reg, viewer_t *viewer)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, scrolling_event_null_reason_code },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            viewer);

  event_set_earliest(os_read_monotonic_time() + 2);
}

static int scrolling_event_null_reason_code(wimp_event_no event_no,
                                            wimp_block   *block,
                                            void         *handle)
{
  viewer_t           *viewer;
  union
  {
    wimp_window_state state;
    wimp_open         open;
  }
  u;
  int                 x, y, s;

  NOT_USED(event_no);
  NOT_USED(block);

  viewer = handle;

/* fprintf(stderr,"scrolling\n"); */

  if (viewer->scrolling.count-- == 0)
  {
    scrolling_set_handlers(0, viewer);
    return event_HANDLED;
  }

  u.state.w = viewer->main_w;
  wimp_get_window_state(&u.state);

  x = viewer->scrolling.pos_x     + viewer->scrolling.dest_x;
  y = viewer->scrolling.pos_y     + viewer->scrolling.dest_y;
  s = viewer->scrolling.pos_scale + viewer->scrolling.dest_scale;
  viewer->scrolling.pos_x     = x;
  viewer->scrolling.pos_y     = y;
  viewer->scrolling.pos_scale = s;

  /* this scale then set order is probably important */

  viewer_scaledlg_set(viewer, s / ScrollingScale, 1);

  /* 'viewer' is possibly stale here now */

  u.open.xscroll = x / ScrollingScale;
  u.open.yscroll = y / ScrollingScale;

  wimp_open_window(&u.open);

  /* now test whether where we've opened is the final resting position, if
   * so cancel any further work */

  return event_HANDLED;
}

static void scrolling_start(viewer_t *viewer,
                            int       dx,
                            int       dy,
                            int       ds,
                            int       delta,
                            int       steps)
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
  scrolling_event_null_reason_code(wimp_NULL_REASON_CODE, NULL, viewer);

  scrolling_set_handlers(1, viewer);
}

/* ----------------------------------------------------------------------- */

/* A strcmp() guaranteed to return -1, 0 or 1. */
static int step_strcmp(const char *a, const char *b)
{
  int r;

  r = strcmp(a, b);

  return r < 0 ? -1 :
         r > 0 ? +1 :
                  0;
}

/*
 * Step to an adjacent disc file.
 */
static int step(viewer_t *viewer, int direction)
{
  const char *leaf_name;
  const char *dir_name;
  int         context;
  char        file_name[256]; /* Careful Now */
  int         read_count;
  int         found; /* bool */
  osgbpb_info_stamped found_info[11];

  if (direction > 0)
    direction = +1;
  else
    direction = -1;

  leaf_name = str_leaf(viewer->drawable->image->file_name);
  dir_name  = str_branch(viewer->drawable->image->file_name);

  /* Enumerate the entire directory to find the previous or next file.
   *
   * While FileCore FS directory indices are stable and we are returned
   * sorted directories we can't make any assumptions about directory
   * ordering on other file systems. So to find the lexicographical
   * previous or next file in a directory we have to read in the entire
   * directory contents. To do that we check to see if the current
   * filename is less than the target filename (when stepping backwards).
   * If so we store it. We read further filenames. If they're less than the
   * target and greater than the stored filename we update the stored
   * filename. At the end of the directory we will have stored the closest
   * matching "less than" filename from the directory. Reversing the signs
   * makes the same process work for stepping forwards.
   */
  found   = 0;
  context = 0;
  do
  {
    char   buffer[256];
    char  *bufp;
    size_t len;

    /* Read in as many directory entries as will fit in the buffer */
    EC(xosgbpb_dir_entries_info_stamped(dir_name,
                                        (osgbpb_info_stamped_list *) buffer,
                                        INT_MAX, /* count */
                                        context,
                                        sizeof(buffer), /* size */
                                        NULL, /* match any filename */
                                        &read_count,
                                        &context));

    /* Process the buffer entries */
    for (bufp = buffer; read_count-- > 0; bufp += (len + 3) & ~3)
    {
      const osgbpb_info_stamped *info;

      info = (const osgbpb_info_stamped *) bufp;
      len  = osgbpb_SIZEOF_INFO_STAMPED(strlen(info->name) + 1); /* note: used in outer for() */

      /* Save any entry which lies between the target and the current
       * best match */
      if (image_is_loadable(info->file_type) &&
          step_strcmp(info->name, leaf_name) == direction)
      {
        if (found == 0 ||
            step_strcmp(info->name, found_info->name) != direction)
        {
          memcpy(found_info, info, len);
          found = 1;
        }
      }
    }
  }
  while (context > -1);

  if (!found)
  {
    /* beep or something */
    return -1;
  }

  /* now we've got a file, and it's loadable */

  if (!viewer_query_unload(viewer))
    return 0;

  viewer_unload(viewer);

  sprintf(file_name, "%s.%s", dir_name, found_info->name);
  if (viewer_load(viewer, file_name, found_info->load_addr, found_info->exec_addr))
  {
    viewer_destroy(viewer);
    return 0;
  }

  viewer_open(viewer);
  return 0;
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler grab_event_user_drag_box,
                          grab_event_pollword_non_zero;

static int grabx, graby;
static int grabminx, grabminy;

static void grab_set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX,     grab_event_user_drag_box     },
    { wimp_POLLWORD_NON_ZERO, grab_event_pollword_non_zero },
  };

/*  printf("grab_set_handlers %d\n", reg); */

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            NULL);
}

static void grab_end(void *opaque)
{
  NOT_USED(opaque);

/*  printf("grab_end\n"); */

  restore_pointer_shape();

  grab_set_handlers(0);
}

static int grab_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
{
  wimp_pointer    *pointer;
  viewer_t        *viewer;
  wimp_window_info info;
  int              x,y;
  wimp_drag        drag;
  int              xvis,yvis;
  int              xscr,yscr;
  int              xext,yext;

/*  printf("grab_event_mouse_click\n"); */

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

  _swi(0x4D942 /* AppEngine_WindowOp */,
       _IN(0)|_OUTR(0,2),
       8,
       &drag.draw,
       &drag.undraw,
       &drag.redraw);

  wimp_drag_box(&drag);

  GLOBALS.dragging_viewer = viewer;

  set_pointer_shape("ptr_grab", 4, 4);

  grab_set_handlers(1);

  dragcancel_set_handler(1, grab_end, NULL);

  return event_HANDLED;
}

static int grab_event_user_drag_box(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

/*  printf("grab_event_user_drag_box\n"); */

  dragcancel_set_handler(0, grab_end, NULL);

  grab_end(NULL);

  return event_HANDLED;
}

static int grab_event_pollword_non_zero(wimp_event_no event_no,
                                        wimp_block   *block,
                                        void         *handle)
{
  wimp_pointer        pointer;
  union
  {
    wimp_window_state state;
    wimp_open         open;
  }
  u;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  event_zero_pollword();

  wimp_get_pointer_info(&pointer);

  u.state.w = GLOBALS.dragging_viewer->main_w;
  wimp_get_window_state(&u.state);

  u.open.xscroll = grabminx + grabx - pointer.pos.x;
  u.open.yscroll = grabminy + graby - pointer.pos.y;

  wimp_open_window(&u.open);

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static int display_event_close_window_request(wimp_event_no event_no,
                                              wimp_block   *block,
                                              void         *handle)
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

  info.w = GLOBALS.current_viewer->main_w;
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

  info.w = GLOBALS.current_viewer->main_w;
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
  viewer_scaledlg_set(GLOBALS.current_viewer->main_w, new_scale, 0);

  info.xscroll = (wax * new_scale / old_scale) - visible_w / 2;
  info.yscroll = (way * new_scale / old_scale) + visible_h / 2;
  wimp_open_window((wimp_open *) &info);

  window_redraw((int) GLOBALS.current_viewer->main_w);
#else
  wax = (wax * new_scale / old_scale);
  way = (way * new_scale / old_scale);

  scrolling_start(viewer, wax, way, new_scale, 0,
                  GLOBALS.choices.viewer.steps);
#endif
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler embed_event_user_drag_box;

static void embed_set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX, embed_event_user_drag_box },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            NULL);
}

static void embed_end(void *opaque)
{
  NOT_USED(opaque);

  restore_pointer_shape();

  embed_set_handlers(0);
}

static int embed_event_mouse_click(wimp_event_no event_no,
                                   wimp_block   *block,
                                   void         *handle)
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

  embed_set_handlers(1);

  dragcancel_set_handler(1, embed_end, NULL);

  return event_HANDLED;
}

static int embed_event_user_drag_box(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
{
  wimp_pointer     pointer;
  wimp_w           parent;
  wimp_window_info info;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  dragcancel_set_handler(0, embed_end, NULL);

  embed_end(NULL);

  wimp_get_pointer_info(&pointer);

  if (pointer.w == GLOBALS.current_viewer->main_w)
    return event_HANDLED; /* can't embed in self */

  parent = (pointer.w == wimp_ICON_BAR) ? wimp_TOP : pointer.w;

  info.w = GLOBALS.current_viewer->main_w;
  wimp_get_window_info_header_only(&info);

  wimp_open_window_nested((wimp_open *) &info,
                          parent,
                          wimp_CHILD_LINKS_PARENT_WORK_AREA);

  return event_HANDLED;
}

/* ---------------------------------------------------------------------- */

static event_wimp_handler zoombox_event_user_drag_box;

static void zoombox_set_handlers(int reg, viewer_t *viewer)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX, zoombox_event_user_drag_box },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            event_ANY_WINDOW,
                            event_ANY_ICON,
                            viewer);
}

static void zoombox_end(void *opaque)
{
  /* Turn off autoscrolling */
  if (GLOBALS.wimp_version >= 399) // should 399 be wimp_VERSION_RO40?
    wimp_auto_scroll(0, NULL);

  restore_pointer_shape();

  zoombox_set_handlers(0, opaque);
}

static int zoombox_event_mouse_click(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
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
    wimp_auto_scroll_info scrinfo;

    scrinfo.w = pointer->w;
    scrinfo.pause_zone_sizes.x0 =
    scrinfo.pause_zone_sizes.y0 =
    scrinfo.pause_zone_sizes.x1 =
    scrinfo.pause_zone_sizes.y1 = 16;
    scrinfo.pause_duration      = 0; /* don't pause */
    scrinfo.state_change        = wimp_AUTO_SCROLL_NO_HANDLER; /* using own pointer */
    /* scrinfo.handle = 0; N/A */

    wimp_auto_scroll(wimp_AUTO_SCROLL_ENABLE_HORIZONTAL |
                     wimp_AUTO_SCROLL_ENABLE_VERTICAL,
                    &scrinfo);

    wimp_drag_box_with_flags(&drag,
                              wimp_DRAG_BOX_KEEP_IN_LINE |
                              wimp_DRAG_BOX_CLIP);
  }
  else
  {
    wimp_drag_box(&drag);
  }

  set_pointer_shape("ptr_mag", 10, 9);

  zoombox_set_handlers(1, viewer);

  dragcancel_set_handler(1, zoombox_end, viewer);

  return event_HANDLED;
}

static int zoombox_event_user_drag_box(wimp_event_no event_no,
                                       wimp_block   *block,
                                       void         *handle)
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

  dragcancel_set_handler(0, zoombox_end, viewer);

  zoombox_end(viewer);

  /* Read the window coordinates */
  info.w = GLOBALS.current_viewer->main_w;
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
      viewer_scaledlg_set(viewer, new_scale, 0);

      /* Set middle point of user's selection (ish) */
      info.xscroll = x0 * new_scale / old_scale;
      info.yscroll = y1 * new_scale / old_scale;

      wimp_open_window((wimp_open *) &info);

      window_redraw(GLOBALS.current_viewer->main_w);
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

/* idea for more generic menu setup

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
                                  metadata_available,
                                  hist_available,
                                  NULL };

static
menutest_entry edit_entries[] = { effects_available,
                                  rotate_available,
                                  to_spr_available,
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

static void display_menu_update(void)
{
  viewer_t        *viewer;
  image_t         *image;
  wimp_menu_entry *entries;
  wimp_menu       *m;

  viewer = GLOBALS.current_viewer;
  if (viewer == NULL)
    return;

  image = viewer->drawable->image;

  entries = GLOBALS.image_m->entries;

  /* Misc menu */

  m = entries[IMAGE_FILE].sub_menu;

  /* Shade the "Histogram" entry if we don't have that method */
  disable(m, FILE_HIST, hist_available(image));

#ifdef EYE_META
  /* Shade the "Metadata" entry if we don't have that method */
  disable(m, FILE_METADATA, metadata_available(image));
#endif

  /* Edit menu */

  m = entries[IMAGE_EDIT].sub_menu;

  /* Shade the "Effects" entry if effects module says "no" */
  disable(m, EDIT_EFFECTS, effects_available(image));

  /* Shade the "Rotate" entry if rotate module says "no" */
  disable(m, EDIT_ROTATE,  rotate_available(image));

  /* Shade the "Convert to Sprite" entry if to_spr module says "no" */
  disable(m, EDIT_CONVERT_TO_SPRITE, to_spr_available(image));

  /* Shade the "Release clipboard" entry if we don't own the clipboard */
  disable(m, EDIT_RELEASE, clipboard_own());
}


static int display_event_mouse_click(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
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

  GLOBALS.current_viewer = viewer;

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
    display_menu_update();

    menu_open(GLOBALS.image_m, pointer->pos.x - 64, pointer->pos.y);
  }
  else if (pointer->buttons & wimp_CLICK_SELECT)
  {
    switch (keys)
    {
      case KeyShift:
        zoom_to_point(pointer, viewer);
        break;

      case KeyCtrl:
        pan_to_point(pointer, viewer);
        break;

      default:
        break;
    }
  }
  else if (pointer->buttons & wimp_CLICK_ADJUST)
  {
    switch (keys)
    {
      case KeyShift:
        zoom_to_point(pointer, viewer);
        break;

      default:
        break;
    }
  }
  else if (pointer->buttons & wimp_DRAG_SELECT)
  {
    switch (keys)
    {
      case KeyNone:
        grab_event_mouse_click(event_no, block, handle);
        break;

      case KeyShiftCtrl:
        embed_event_mouse_click(event_no, block, handle);
        break;

      default:
        break;
    }
  }
  else if (pointer->buttons & wimp_DRAG_ADJUST)
  {
    switch (keys)
    {
      case KeyNone:
        zoombox_event_mouse_click(event_no, block, handle);
        break;

      default:
        break;
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

  viewer_scaledlg_set(viewer, new_scale, 1);
}

static void action_step(viewer_t *viewer, int op)
{
  (void) step(viewer, (op == StepForwards) ? 1 : -1);
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
    scrolling_event_null_reason_code(wimp_NULL_REASON_CODE, NULL, viewer);
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

  oserror_report_block(e);
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
    result_report(action_close_window(viewer->main_w));
    break;

  case Save:
    dialogue_show(viewer_savedlg);
    break;

  case Info:
    dialogue_show(viewer_infodlg);
    break;

  case SourceInfo:
    dialogue_show(viewer_srcinfodlg);
    break;

  case Hist:
    hist_open(viewer->drawable->image, GLOBALS.choices.hist.bars);
    break;

  case Rotate:
    rotate_open(viewer->drawable->image);
    break;

  case Scale:
    dialogue_show(viewer_scaledlg);
    break;

#ifdef EYE_TAGS
  case Tags:
    result_report(tags_open(viewer->drawable->image));
    break;
#endif

  case Copy:
    clipboard_claim(viewer->main_w);
    break;

  case Help:
    result_report(action_help());
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
    effects_open(viewer->drawable->image);
    break;

  case MetaData:
#ifdef EYE_META
    metadata_open(viewer->drawable->image,
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

static int display_event_key_pressed(wimp_event_no event_no,
                                     wimp_block   *block,
                                     void         *handle)
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

  op = viewer_keymap_op(LOCALS.keymap_id, key->c);
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

static int display_event_menu_selection(wimp_event_no event_no,
                                        wimp_block   *block,
                                        void         *handle)
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
    { PACK(IMAGE_FILE, FILE_METADATA),          MetaData  },
#endif
    { PACK(IMAGE_FILE, FILE_HIST),              Hist      },
#ifdef EYE_TAGS
    { PACK(IMAGE_FILE, FILE_TAGS),              Tags      },
#endif
    { PACK(IMAGE_FILE, FILE_NEWVIEW),           NewView   },
    { PACK(IMAGE_EDIT, EDIT_EFFECTS),           Effects   },
    { PACK(IMAGE_EDIT, EDIT_ROTATE),            Rotate    },
    { PACK(IMAGE_EDIT, EDIT_CONVERT_TO_SPRITE), ConvToSpr },
    { PACK(IMAGE_EDIT, EDIT_CLAIM),             Copy      },
    { PACK(IMAGE_EDIT, EDIT_RELEASE),           Release   },
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
    viewer = GLOBALS.current_viewer;
    if (viewer == NULL)
      return event_HANDLED;

    action(viewer, map[i].op);
  }

  wimp_get_pointer_info(&p);
  if (p.buttons & wimp_CLICK_ADJUST)
  {
    display_menu_update();
    menu_reopen();
  }

  return event_HANDLED;

#undef PACK
}

static int display_event_scroll_request(wimp_event_no event_no,
                                        wimp_block   *block,
                                        void         *handle)
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

static int display_event_lose_caret(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  viewer_t *viewer;

  NOT_USED(event_no);
  NOT_USED(block);

  viewer = handle;

  image_defocus(viewer->drawable->image);

  return event_HANDLED;
}

static int display_event_gain_caret(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  viewer_t *viewer;

  NOT_USED(event_no);
  NOT_USED(block);

  viewer = handle;

  image_focus(viewer->drawable->image);

  return event_HANDLED;
}
