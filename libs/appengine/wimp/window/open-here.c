/* $Id: open-here.c,v 1.2 2009-05-04 22:29:38 dpt Exp $ */

#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

#include "appengine/wimp/window/open.h" /* private header */

/* ----------------------------------------------------------------------- */

struct Args
{
  const os_box         *box;
  window_open_at_flags  flags;
};

static void open_here_cb(CallbackArgs *sizes, void *arg)
{
  struct Args *args;
  int          x, y;
  int          w, h;
  int          width, height;
  int          scr_w, scr_h;
  int          over_x, over_y;

  args = arg;

  x = args->box->x0 - sizes->left;
  y = args->box->y0 - sizes->bottom;

  w = args->box->x1 - args->box->x0;
  h = args->box->y1 - args->box->y0;

  width  = sizes->left + w + sizes->right;
  height = sizes->top  + h + sizes->bottom;

  scr_w = sizes->screen_w;
  scr_h = sizes->screen_h;

  /* compensate for icon bar size */

  if (args->flags & AT_NOCOVERICONBAR)
    scr_h -= IconBarHeight - IconBarOverlap;

  /* allow the window to be no larger than the screen */

  if (width  > scr_w) width  = scr_w;
  if (height > scr_h) height = scr_h;

  /* ensure it's not off the bottom/left of the screen */

  if (x < 0) x = 0;
  if (y < 0) y = 0;

  /* if it's off the top/right of the screen, move it left */

  over_x = x + width  - scr_w;
  over_y = y + height - scr_h;
  if (over_x > 0) x -= over_x;
  if (over_y > 0) y -= over_y;

  /* compensate for icon bar size, if we obscure it */

  if (args->flags & AT_NOCOVERICONBAR && y < IconBarHeight - IconBarOverlap)
    y += IconBarHeight - IconBarOverlap;

  /* we've included the window borders in our calculations, now remove */

  sizes->info.visible.x0 =   sizes->left   + x;
  sizes->info.visible.y0 =   sizes->bottom + y;
  sizes->info.visible.x1 = - sizes->right  + x + width;
  sizes->info.visible.y1 = - sizes->top    + y + height;

  centre_scrollbars(&sizes->info);
}

void window_open_here_flags(wimp_w                w,
                      const os_box               *box,
                            window_open_at_flags  flags)
{
  struct Args args;

  args.box   = box;
  args.flags = flags;

  window_open_with_callback(w, open_here_cb, &args);
}
