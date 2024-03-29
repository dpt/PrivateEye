// #include <stdio.h>

#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"

#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/wimp/window/open.h" /* private header */

/* ----------------------------------------------------------------------- */

enum
{
  StaggerOffset = 48
};

/* ----------------------------------------------------------------------- */

void window_open_with_callback(wimp_w w,
                               void (*cb)(window_open_args_t *, void *),
                               void  *opaque)
{
  static const os_VDU_VAR_LIST(5) var_list =
  {{
    os_MODEVAR_XEIG_FACTOR,
    os_MODEVAR_YEIG_FACTOR,
    os_MODEVAR_XWIND_LIMIT,
    os_MODEVAR_YWIND_LIMIT,
    -1,
  }};

  int                vdu_vars[4];
  window_open_args_t args;

  os_read_vdu_variables((const os_vdu_var_list *) &var_list, vdu_vars);

  args.screen_w = (vdu_vars[2] + 1) << vdu_vars[0];
  args.screen_h = (vdu_vars[3] + 1) << vdu_vars[1];

  args.info.w = w;
  wimp_get_window_info_header_only(&args.info);

  /* calculate furniture sizes */

  /* avoid calling wimp_get_window_outline() on unopened windows if we're
   * on Wimps earlier than the nested Wimp */
  if ((args.info.flags & wimp_WINDOW_OPEN) != 0 ||
      (wimpreadsysinfo_version() >= wimp_VERSION_RO38))
  {
    args.outline.w = w;
    wimp_get_window_outline(&args.outline);

    args.left   = args.info.visible.x0 - args.outline.outline.x0;
    args.bottom = args.info.visible.y0 - args.outline.outline.y0;
    args.right  = args.outline.outline.x1 - args.info.visible.x1;
    args.top    = args.outline.outline.y1 - args.info.visible.y1;
  }
  else
  {
    // newlook: 12x11 at 45x45dpi,  22x11 at 90x45dpi, 20x20 at 90x90dpi
    // RO3.1: 21x11 on RO3.1 at 90x45dpi

    /* assuming 40 OS units */
    const int StandardToolSpriteWidth  = 40;
    const int StandardToolSpriteHeight = 40;

    args.left   = args.right = 1 << vdu_vars[0];
    args.bottom = args.top   = 1 << vdu_vars[1];

    if (args.info.flags & wimp_WINDOW_TITLE_ICON)
      args.top = StandardToolSpriteHeight;
    if (args.info.flags & wimp_WINDOW_VSCROLL)
      args.right = StandardToolSpriteWidth;
    if (args.info.flags & wimp_WINDOW_HSCROLL)
      args.bottom = StandardToolSpriteHeight;
  }

  // fprintf(stderr, "furn: %d %d %d %d\n", args.left, args.bottom, args.right, args.top);

  cb(&args, opaque);

  wimp_open_window((wimp_open *) &args.info);
}

/* ----------------------------------------------------------------------- */

void centre_scrollbars(wimp_window_info *info)
{
  int vw, vh;
  int ew, eh;

  vw = info->visible.x1 - info->visible.x0;
  vh = info->visible.y1 - info->visible.y0;

  ew = info->extent.x1 - info->extent.x0;
  eh = info->extent.y1 - info->extent.y0;

  info->xscroll = info->extent.x0 + (ew - vw) / 2;
  info->yscroll = info->extent.y1 - (eh - vh) / 2;
}

/* ----------------------------------------------------------------------- */

static void open_at_cb(window_open_args_t *args, void *opaque)
{
  unsigned int where;
  unsigned int flags;
  os_box      *ext;
  int          ext_w, ext_h;
  int          width, height;
  int          scr_w, scr_h;
  int          x, y;

  where = (unsigned int) opaque;
  flags = where & (AT_FORCE | AT_NOCOVERICONBAR | AT_USEVISIBLEAREA);
  where ^= flags;

  if ((args->info.flags & wimp_WINDOW_OPEN) != 0 &&
      (flags & AT_FORCE) == 0)
  {
    /* already open - reopen on top at its present size */
    args->info.next = wimp_TOP;
    return;
  }

  if (where == AT_DEFAULT)
    return;

  /* We're positioning the visible area of the window, ignoring any
   * borders, but that means if we get a window the same size or larger
   * than one of the screen's dimensions AND we're re-opening a window
   * (not opening it fresh) then the window will be moved off screen,
   * since the Wimp forces windows on-screen ONLY on the first open.
   */

  if (flags & AT_USEVISIBLEAREA)
    ext = &args->info.visible;
  else
    ext = &args->info.extent; /* default */

  ext_w = ext->x1 - ext->x0;
  ext_h = ext->y1 - ext->y0;

  width  = args->left + ext_w + args->right;
  height = args->top  + ext_h + args->bottom;

  scr_w = args->screen_w;
  scr_h = args->screen_h;

  /* allow the window to be no larger than the screen */
  if (width  > scr_w) width  = scr_w;
  if (height > scr_h) height = scr_h;

  x = 0; /* defaults */
  y = 0;

  /* Horizontal position */
  switch ((where & AT_HMASK) >> AT_H)
  {
    case AT_CEN:    x = (args->screen_w - width) / 2; break;
    case AT_LEFTOP: x = 0;                            break;
    case AT_RIGBOT: x = args->screen_w - width;       break;
  }

  /* Vertical position */
  switch ((where & AT_VMASK) >> AT_V)
  {
    case AT_CEN:    y = (args->screen_h - height) / 2; break;
    case AT_LEFTOP: y = args->screen_h - height;       break;
    case AT_RIGBOT: y = read_icon_bar_height();        break;
  }

  /* Other positions */
  switch (where)
  {
    /* Open centred on the pointer taking care to not obscure the icon bar */

    case AT_BOTTOMPOINTER:
      {
        wimp_pointer p;
        int          min_y;

        wimp_get_pointer_info(&p);

        x = p.pos.x - width / 2;
        y = p.pos.y - height / 2;

        min_y = read_icon_bar_height(); /* exactly above the icon bar, like DisplayManager */
        if (y < min_y)
          y = min_y;
      }
      break;

    case AT_STAGGER:
      {
        static int lastopen_x = -1, lastopen_y = -1;

        /* open centred, then offset subsequently */
        if (lastopen_x == -1 && lastopen_y == -1)
        {
          /* new stagger, centre initially */
          x = (args->screen_w - width) / 2;
          y = (args->screen_h - height) / 2;
        }
        else
        {
          /* move progressively down, then across */
          x = lastopen_x;
          y = lastopen_y - StaggerOffset;
          if (y < read_icon_bar_height()) /* above the icon bar */
          {
            x += StaggerOffset;
            if (x + width >= args->screen_w)
              x = StaggerOffset;
            y = args->screen_h - height - StaggerOffset;
          }
        }
        lastopen_x = x;
        lastopen_y = y;
      }
      break;
  }

  /* avoid covering the icon bar, if we would obscure it */
  if (flags & AT_NOCOVERICONBAR)
  {
    int ibh;
    
    ibh = read_icon_bar_unobscured();
    if (y < ibh)
    {
      y      = ibh;
      height = MIN(height, scr_h - ibh);
    }
  }

  /* we've included the window borders in our calculations, now remove */
  args->info.visible.x0 =   args->left   + x;
  args->info.visible.y0 =   args->bottom + y;
  args->info.visible.x1 = - args->right  + x + width;
  args->info.visible.y1 = - args->top    + y + height;

  centre_scrollbars(&args->info);
}

void window_open_at(wimp_w w, window_open_at_flags where)
{
  window_open_with_callback(w, open_at_cb, (void *) where /* grisly */);
}
