/* --------------------------------------------------------------------------
 *    Name: capture.c
 * Purpose: window_capture, window_restore
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/vdu/screen.h"

#include "appengine/wimp/window.h"

/* Just checking wimp_WINDOW_FULL_SIZE isn't sufficient for our needs since
 * we have our own 'no cover icon bar' flag distinct from the Wimp's. */
static osbool window_is_maximised(wimp_w win, osbool covericonbar)
{
  wimp_window_info info;
  wimp_outline outline;
  int sw,sh;
  int h;

  info.w = win;
  wimp_get_window_info_header_only(&info);

  if ((info.flags & wimp_WINDOW_OPEN) == 0)
    return FALSE;

  if (info.flags & wimp_WINDOW_FULL_SIZE)
    return TRUE;

  /* The window might be maximised from our point of view (ie. fill the whole
   * desktop area save for icon bar) but not actually be marked as full size
   * by the WindowManager. */

  outline.w = win;
  wimp_get_window_outline(&outline);

  h = outline.outline.y1 - outline.outline.y0;

  read_screen_dimensions(&sw, &sh);

  /* The test is exact. We could be a bit more generous and allow anything
   * which overlaps the bottom of the icon bar to be considered maximised. */

  if (!covericonbar)
    if (h == (sh - read_icon_bar_unobscured()))
      return TRUE;

  return FALSE;
}

/* remember current window position for when we position window later */
void window_capture(wimp_w w, WindowCapture *capture, osbool covericonbar)
{
  wimp_window_info info;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  capture->cx = info.visible.x0 + (info.visible.x1 - info.visible.x0) / 2;
  capture->cy = info.visible.y0 + (info.visible.y1 - info.visible.y0) / 2;
  capture->flags = info.flags;

  /* Alternatively could store more details removing the need to take
   * covericonbar as a parameter here. */

  if (window_is_maximised(w, covericonbar))
    capture->flags |= wimp_WINDOW_FULL_SIZE; /* force set */
}

void window_restore(wimp_w win, WindowCapture *capture, osbool covericonbar)
{
  wimp_window_info info;
  unsigned int     flags;

  /*fprintf(stderr, "opening %p", win);*/

  info.w = win;
  wimp_get_window_info_header_only(&info);

  /* Keeping the window on-screen is tricky because the Wimp will not force
   * already-open windows to the size of the screen. The window_open routines
   * handle this for us. */

  if (capture->flags & wimp_WINDOW_OPEN)
  {
    /* window was *previously* open:
     * - if maximised, keep maximised, otherwise use present size
     * - don't move in window stack
     */
    os_box *ext;
    int     w,h;
    os_box  box;

    /*fprintf(stderr, " previously open\n");*/

    if (capture->flags & wimp_WINDOW_FULL_SIZE)
      ext = &info.extent;
    else
      ext = &info.visible;

    w = ext->x1 - ext->x0;
    h = ext->y1 - ext->y0;

    box.x0 = capture->cx - w / 2;
    box.y0 = capture->cy - h / 2;
    box.x1 = box.x0 + w;
    box.y1 = box.y0 + h;

    flags = 0;
    if (!covericonbar)
      flags |= AT_NOCOVERICONBAR;

    window_open_here_flags(win, &box, flags);
  }
  else
  {
    /* window was *previously* closed:
     * - open maximised
     * - centred on the screen
     * - on top of window stack
     */
    /*fprintf(stderr, " previously closed\n");*/

    flags = AT_CENTRE;
    if (!covericonbar)
      flags |= AT_NOCOVERICONBAR;

    window_open_at(win, flags);
  }
}
