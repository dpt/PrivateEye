/* --------------------------------------------------------------------------
 *    Name: rotate.c
 * Purpose: Rotation
 * Version: $Id: rotate.c,v 1.39 2009-09-13 16:21:58 dpt Exp $
 * ----------------------------------------------------------------------- */

/* TODO
 *
 * - Detect when dialogue closes, free pixtrans tables.
 * - Fix that snapping routine.
 * - Find the cause of the not-always-snapping bug.
 * - Reference sprites as pointers.
 * - Avoid flicker.
 */

#include "swis.h"

#include <math.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/colourtrans.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/osword.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/os.h"
#include "appengine/geom/trfm.h"
#include "appengine/graphics/thumbnail.h"
#include "appengine/vdu/screen.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/pointer.h"

#include "globals.h"
#include "rotate.h"
#include "iconnames.h"

enum
{
  /* window arrangement values */

  GAP           = 8, /* gap between icons / window edges */
  ROTATEW       = 204,
  CANCELW       = 188,
  ROTATEX       = - (ROTATEW + GAP),       /* from RHS */
  ROTATEY       = GAP,
  CANCELX       = ROTATEX - GAP - CANCELW, /* from RHS */
  CANCELY       = ROTATEY + 8,
  BUTTON_HEIGHT = 68 + 2 * GAP,
  MIN_WIDTH     = 416,
};

enum
{
  RotateFlag_HorzFlip = 1 << 0
};
typedef unsigned int RotateFlags;

static struct
{
  wimp_w        rotate_w;

  image       *image;

  RotateFlags  flags;
  int          rotation;       /* basic / final rotation */

  int          start;          /* start angle of current rotate op */
  int          delta;          /* angle of current rotate op */

  RotateFlags  drawn_flags;
  int          drawn_rotation; /* rotation last drawn */

  int          extent;         /* largest dimension (x/y same) */
  int          cx,cy;          /* centre of rotation */
  os_box       redraw_area;

  osspriteop_area *thumbnail;

  osspriteop_trans_tab *trans_tab; /* for thumbnail */

  struct
  {
    osspriteop_trans_tab *trans_tab;
  }
  indicator;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static event_wimp_handler rotate__event_redraw_window_request,
                          rotate__event_mouse_click,
                          rotate__event_user_drag_box,
                          rotate__event_pollword_non_zero;

static event_message_handler rotate__message_menus_deleted;

/* ----------------------------------------------------------------------- */

static void rotate__set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_REDRAW_WINDOW_REQUEST, rotate__event_redraw_window_request },
    { wimp_MOUSE_CLICK,           rotate__event_mouse_click           },
  };

  static const event_message_handler_spec message_handlers[] =
  {
    { message_MENUS_DELETED, rotate__message_menus_deleted },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            LOCALS.rotate_w, event_ANY_ICON,
                            NULL);

  event_register_message_group(reg,
                               message_handlers, NELEMS(message_handlers),
                               LOCALS.rotate_w, event_ANY_ICON,
                               NULL);
}

error rotate__init(void)
{
  error err;

  /* dependencies */

  err = help__init();
  if (err)
    return err;

  LOCALS.rotate_w = window_create("rotate");

  rotate__set_handlers(1);

  err = help__add_window(LOCALS.rotate_w, "rotate");
  if (err)
    return err;

  return error_OK;
}

void rotate__fin(void)
{
  help__remove_window(LOCALS.rotate_w);

  rotate__set_handlers(0);

  help__fin();
}

static void rotate__set_drag_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_USER_DRAG_BOX,     rotate__event_user_drag_box     },
    { wimp_POLLWORD_NON_ZERO, rotate__event_pollword_non_zero },
  };

  event_register_wimp_group(reg,
                            wimp_handlers, NELEMS(wimp_handlers),
                            event_ANY_WINDOW, event_ANY_ICON,
                            NULL);
}

/* ----------------------------------------------------------------------- */

static void shift_buttons(const os_box *box)
{
  static const struct
  {
    wimp_i i;
    int    x,y;
  }
  offsets[] =
  {
    { ROTATE_B_ROTATE, ROTATEX, ROTATEY },
    { ROTATE_B_CANCEL, CANCELX, CANCELY }
  };
  int x,y;
  int i;

  x = box->x1;
  y = box->y0;

  for (i = 0; i < NELEMS(offsets); i++)
  {
    move_icon(LOCALS.rotate_w,
              offsets[i].i,
              x + offsets[i].x,
              y + offsets[i].y);
  }
}

static error make_trans_tab(const osspriteop_area *area,
                            osspriteop_id          source_id,
                            osspriteop_trans_tab **new_trans_tab)
{
  int                   size;
  osspriteop_trans_tab *trans_tab;

  *new_trans_tab = NULL;

  size = colourtrans_generate_table_for_sprite(area,
                                               source_id,
                                               colourtrans_CURRENT_MODE,
                                               colourtrans_CURRENT_PALETTE,
                                               NULL, /* trans_tab */
                                               0, /* flags */
                                               NULL,
                                               NULL);
  if (size == 0)
    return error_OK;

  trans_tab = malloc(size);
  if (trans_tab == NULL)
    return error_OOM;

  size = colourtrans_generate_table_for_sprite(area,
                                               source_id,
                                               colourtrans_CURRENT_MODE,
                                               colourtrans_CURRENT_PALETTE,
                                               trans_tab,
                                               0, /* flags */
                                               NULL,
                                               NULL);

  *new_trans_tab = trans_tab;

  return error_OK;
}

static void rotate__close_dialogue(void)
{
  wimp_create_menu(wimp_CLOSE_MENU, 0, 0);

  /* This will cause a message_MENUS_DELETED to be received, freeing the
   * thumbnail created earlier on. */
}

/* build stuff necessary for rotate dialogue */
static void rotate__update_dialogue(void)
{
  error             err;
  int               w,h;
  int               d;
  int               extent;
  osbool            mask;
  os_mode           mode;
  int               xeig,yeig;
  os_box            box;
  int               cx,cy;
  int               r;

  /* ensure we have a thumbnail ready */

  if (LOCALS.thumbnail == NULL)
    if (thumbnail_create(LOCALS.image,
                        &GLOBALS.choices.drawable,
                         GLOBALS.choices.rotate.max_thumb,
                        &LOCALS.thumbnail))
      return; /* failure */

  /* read thumbnail details */

  osspriteop_read_sprite_info(osspriteop_USER_AREA,
                              LOCALS.thumbnail,
             (osspriteop_id) "thumbnail",
                             &w,&h,
                             &mask,
                             &mode);

  /* convert dimensions to OS units */

  read_mode_vars(mode, &xeig, &yeig, NULL);

  w <<= xeig;
  h <<= yeig;

  /* get max dimension */

  d = ((w > h) ? w : h) * 362 / 256; /* 1.41421356 is approx 362/256 */

  /* enforce a minimum dimension so the buttons don't go outside the
   * window bounds */

  extent = (d < MIN_WIDTH) ? MIN_WIDTH : d;

  box.x0 =  0;
  box.y0 = -extent - BUTTON_HEIGHT;
  box.x1 =  extent;
  box.y1 =  0;
  window_set_submenu_extent(LOCALS.rotate_w, 15, &box);

  /* cache values */

  LOCALS.rotation = 0;
  LOCALS.flags    = 0;
  LOCALS.extent   = extent;

  /* the box formed by extent squared may be bigger than required due to
   * minimum size padding. centre our refresh area in this box. */

  cx =  extent / 2;
  cy = -extent / 2;
  r  = d / 2;

  LOCALS.redraw_area.x0 = cx - r;
  LOCALS.redraw_area.y0 = cy - r;
  LOCALS.redraw_area.x1 = cx + r;
  LOCALS.redraw_area.y1 = cy + r;

  /* adjust button positions */

  shift_buttons(&box);


  if (LOCALS.trans_tab)
  {
    free(LOCALS.trans_tab);
    LOCALS.trans_tab = NULL;
  }

  err = make_trans_tab(LOCALS.thumbnail,
       (osspriteop_id) "thumbnail",
                       &LOCALS.trans_tab);
  if (err)
    goto Failure;


  if (LOCALS.indicator.trans_tab)
  {
    free(LOCALS.indicator.trans_tab);
    LOCALS.indicator.trans_tab = NULL;
  }

  err = make_trans_tab((osspriteop_area *) window_get_sprite_area(),
                           (osspriteop_id) "rt-0",
                                           &LOCALS.indicator.trans_tab);
  if (err)
    goto Failure;


  /* must now refresh indicator and thumbnail */

  wimp_force_redraw(LOCALS.rotate_w, 0, -extent, extent, 0);

  return;


Failure:

  thumbnail_destroy(&LOCALS.thumbnail);

  if (LOCALS.trans_tab)
  {
    free(LOCALS.trans_tab);
    LOCALS.trans_tab = NULL;
  }

  if (LOCALS.indicator.trans_tab)
  {
    free(LOCALS.indicator.trans_tab);
    LOCALS.indicator.trans_tab = NULL;
  }

  return;
}

/* idea: use Wimp_PlotIcon instead */
static void draw_indicator(int x, int y)
{
  static const char *sprites[] =
  {
    "rt-0", "rt-90a", "rt-180", "rt-90c",
    "rt-fliph", "rt-transp", "rt-flipv", "rt-transv"
  };
  int a;

  a = LOCALS.drawn_rotation / 65536 + 360;

  if ((a % 90) != 0)
    return;

  a = (a / 90) % 4;
  if (LOCALS.drawn_flags & RotateFlag_HorzFlip)
    a += 4;

  osspriteop_put_sprite_scaled(osspriteop_USER_AREA,
           (osspriteop_area *) window_get_sprite_area(),
               (osspriteop_id) sprites[a],
                               x,y,
                               osspriteop_USE_MASK,
                               NULL,
                               LOCALS.indicator.trans_tab);
}

static void redraw_guts(wimp_draw *draw, osbool (*redraw_fn)(wimp_draw *))
{
  int              more;
  int              w,h;
  osbool           mask;
  os_mode          mode;
  int              xeig,yeig;
  os_trfm          t;

  /* FIXME: Try to factor these calls out - the values shouldn't change
   *        across a rotate 'session'. */

  osspriteop_read_sprite_info(osspriteop_USER_AREA,
                              LOCALS.thumbnail,
              (osspriteop_id) "thumbnail",
                              &w, &h,
                              &mask,
                              &mode);

  read_mode_vars(mode, &xeig, &yeig, NULL);

  w <<= xeig;
  h <<= yeig;

  LOCALS.drawn_rotation = LOCALS.rotation + LOCALS.delta;
  LOCALS.drawn_flags    = LOCALS.flags;

  trfm_set_identity(&t);
  /* flip horizontally */
  if (LOCALS.flags & RotateFlag_HorzFlip)
  {
    t.entries[0][0] = -t.entries[0][0];
    t.entries[2][0] += w;
  }
  trfm_translate(&t, -w / 2, -h / 2);
  trfm_rotate_degs(&t, LOCALS.drawn_rotation);

  t.entries[0][0] *= 256;
  t.entries[0][1] *= 256;
  t.entries[1][0] *= 256;
  t.entries[1][1] *= 256;

  for (more = redraw_fn(draw); more; more = wimp_get_rectangle(draw))
  {
    int     x,y;
    os_trfm t2;

    x = draw->box.x0 - draw->xscroll;
    y = draw->box.y1 - draw->yscroll;

    t2 = t;
    trfm_translate(&t2, x + LOCALS.extent / 2,
                        y - LOCALS.extent / 2);

    t2.entries[2][0] *= 256;
    t2.entries[2][1] *= 256;

    osspriteop_put_sprite_trfm(osspriteop_USER_AREA,
                               LOCALS.thumbnail,
               (osspriteop_id) "thumbnail",
                               0, /* osspriteop_trfm_flags */
                               NULL, /* os_box const *source_rect */
                               mask ? osspriteop_USE_MASK : 0,
                               &t2,
                               LOCALS.trans_tab);

    draw_indicator(x + 8, y - 32 - 8);
  }
}

static int rotate__event_redraw_window_request(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_draw *draw;

  NOT_USED(event_no);
  NOT_USED(handle);

  draw = &block->redraw;

  redraw_guts(draw, wimp_redraw_window);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int get_angle(int x, int y)
{
  double angle;

  if (x != 0 || y != 0)
    angle = (atan2(y, x) / (2 * 3.14159265)) * 360.0 - 90.0;
  else
    angle = 0.0;

  return (int)(angle * 65536.0);
}

static int get_nearest_orthogonal_angle(int angle, int snap) /* aka 'snap' */
{
  angle >>= 16; /* this loses some accuracy ... not sure if i mind */
#if 1
       if (angle >= (-270 - snap) && angle <= (-270 + snap)) angle = -270;
  else if (angle >= (-180 - snap) && angle <= (-180 + snap)) angle = -180;
  else if (angle >=  (-90 - snap) && angle <=  (-90 + snap)) angle =  -90;
  else if (angle >=    (0 - snap) && angle <=    (0 + snap)) angle =    0;
  else if (angle >=   (90 - snap) && angle <=   (90 + snap)) angle =   90;
  else if (angle >=  (180 - snap) && angle <=  (180 + snap)) angle =  180;
  else if (angle >=  (270 - snap) && angle <=  (270 + snap)) angle =  270;
#else
  /* this doesn't work (yet) */
  int t;

  t = (angle + 360 - 45) % 90; /* +360 to ensure +ve */
  if (t >= (45 - snap) && t <= (45 + snap))
    angle = angle / 90 * 90;
#endif
  angle <<= 16;

  return angle;
}

static void refresh(void)
{
  wimp_force_redraw(LOCALS.rotate_w, LOCALS.redraw_area.x0,
                                     LOCALS.redraw_area.y0,
                                     LOCALS.redraw_area.x1,
                                     LOCALS.redraw_area.y1);

  /*draw.w       = LOCALS.rotate_w;
  draw.box.x0  = 0;
  draw.box.y0  = -1000;
  draw.box.x1  = 1000;
  draw.box.y1  = 0;

  redraw_guts(&draw, wimp_update_window);*/


  /* redraw indicator */

  wimp_force_redraw(LOCALS.rotate_w, + 8,
                                     - 8 - 32,
                                     + 8 + 32,
                                     - 8);
}

static int rotate__event_pollword_non_zero(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer pointer;
  int          rotation;

  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  event_zero_pollword();

  wimp_get_pointer_info(&pointer);

  LOCALS.delta = get_angle(pointer.pos.x - LOCALS.cx,
                           pointer.pos.y - LOCALS.cy) - LOCALS.start;

  rotation = LOCALS.rotation + LOCALS.delta;

  rotation = get_nearest_orthogonal_angle(rotation,
                                          GLOBALS.choices.rotate.snap);

  LOCALS.delta = rotation - LOCALS.rotation;

  if (LOCALS.drawn_rotation != rotation ||
      LOCALS.drawn_flags != LOCALS.flags)
  {
    refresh();
  }

  return event_HANDLED;
}

static int rotate__message_menus_deleted(wimp_message *message, void *handle)
{
  wimp_message_menus_deleted *deleted;

  NOT_USED(handle);

  deleted = (wimp_message_menus_deleted *) &message->data;

  thumbnail_destroy(&LOCALS.thumbnail);

  image_stop_editing(LOCALS.image);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static void rotate_internal(image *image, int angle, int hflip)
{
  angle = angle / 65536;
  while (angle < 0) /* safety */
    angle += 360;
  if (hflip)
    angle += 360;

  if (angle)
  {
    int rc;

    rc = image->methods.rotate(&GLOBALS.choices.image, image, angle);

    if (rc)
      oserror__report(0, "error.jpeg.rotate");
  }
}

void rotate(image *image, int angle, int hflip)
{
  if (!rotate__available(image))
  {
    beep();
    return;
  }

  rotate_internal(image, angle, hflip);
}

static int rotate__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case wimp_ICON_WINDOW:
    {
      wimp_window_state state;
      wimp_drag         drag;

      state.w = LOCALS.rotate_w;
      wimp_get_window_state(&state);

      LOCALS.cx = state.visible.x0 + LOCALS.extent / 2;
      LOCALS.cy = state.visible.y1 - LOCALS.extent / 2;

      LOCALS.start = get_angle(pointer->pos.x - LOCALS.cx,
                               pointer->pos.y - LOCALS.cy);
      LOCALS.delta = 0;

      if (pointer->buttons == wimp_CLICK_ADJUST)
        LOCALS.flags = RotateFlag_HorzFlip;
      else
        LOCALS.flags = 0;

      set_pointer_shape((LOCALS.flags & RotateFlag_HorzFlip) ?
                        "ptr_rotflip" : "ptr_rot", 8, 8);

      drag.type       = wimp_DRAG_ASM_FIXED;

      drag.initial.x0 = pointer->pos.x;
      drag.initial.y0 = pointer->pos.y;
      drag.initial.x1 = pointer->pos.x;
      drag.initial.y1 = pointer->pos.y;

      /* Make the parent bounds screen-relative */
      drag.bbox.x0    = state.visible.x0 - state.xscroll;
      drag.bbox.y0    = state.visible.y0 - state.yscroll;
      drag.bbox.x1    = state.visible.x1 - state.xscroll;
      drag.bbox.y1    = state.visible.y1 - state.yscroll;

      drag.handle     = event_get_pollword();
      _swi(0x4D942 /* AppEngine_WindowOp */, _IN(0)|_OUTR(0,2),
           8, &drag.draw, &drag.undraw, &drag.redraw);

      wimp_drag_box(&drag);

      rotate__set_drag_handlers(1);
    }
      break;

    case ROTATE_B_ROTATE:
      rotate_internal(LOCALS.image,
                      LOCALS.rotation,
                     (LOCALS.drawn_flags & RotateFlag_HorzFlip) != 0);
      break;
    }

    if (pointer->i == ROTATE_B_ROTATE || pointer->i == ROTATE_B_CANCEL)
    {
      if (pointer->buttons & wimp_CLICK_SELECT)
        rotate__close_dialogue();
      else
        rotate__update_dialogue();
    }
  }

  return event_HANDLED;
}

static int rotate__event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  restore_pointer_shape();

  rotate__set_drag_handlers(0);

  LOCALS.rotation = (LOCALS.rotation + LOCALS.delta) % (360 << 16);
  LOCALS.delta = 0; /* stop any further movement in redraw */

  /* snap to nearest at this point */

  LOCALS.rotation = get_nearest_orthogonal_angle(LOCALS.rotation, 45);
  refresh();

  return event_HANDLED;
}

void rotate__open(image *image)
{
  if (!rotate__available(image))
  {
    beep();
    return;
  }

  LOCALS.image = image;

  image_start_editing(LOCALS.image);

  rotate__update_dialogue();
  window_open_as_menu(LOCALS.rotate_w);
}

int rotate__available(const image *image)
{
  return image &&
         !image_is_editing(image) &&
         image->flags & image_FLAG_CAN_ROT;
}
