
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/wimp/window/open.h" /* private header */

/* ----------------------------------------------------------------------- */

typedef struct open_here_args
{
  const os_box        *box;
  window_open_at_flags flags;
}
window_open_here_args_t;

static void open_here_cb(window_open_args_t *args, void *opaque)
{
  window_open_here_args_t *here_args;
  int                      x, y;
  int                      w, h;
  int                      width, height;
  int                      scr_w, scr_h;
  int                      over_x, over_y;

  here_args = opaque;

  x = here_args->box->x0 - args->left;
  y = here_args->box->y0 - args->bottom;

  w = here_args->box->x1 - here_args->box->x0;
  h = here_args->box->y1 - here_args->box->y0;

  width  = args->left + w + args->right;
  height = args->top  + h + args->bottom;

  scr_w = args->screen_w;
  scr_h = args->screen_h;

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

  /* avoid covering the icon bar, if we would obscure it */
  if (here_args->flags & AT_NOCOVERICONBAR)
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

void window_open_here_flags(wimp_w               w,
                      const os_box              *box,
                            window_open_at_flags flags)
{
  window_open_here_args_t here_args;

  here_args.box   = box;
  here_args.flags = flags;

  window_open_with_callback(w, open_here_cb, &here_args);
}
