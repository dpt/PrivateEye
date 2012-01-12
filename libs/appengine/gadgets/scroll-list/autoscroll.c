/* --------------------------------------------------------------------------
 *    Name: autoscroll.c
 * Purpose: Scrolling list
 * ----------------------------------------------------------------------- */

#include "oslib/wimp.h"
#include "oslib/wimpreadsysinfo.h"

#include "appengine/wimp/event.h"

#include "appengine/gadgets/scroll-list.h"

#include "impl.h"

void scroll_list_autoscroll(scroll_list *sl, int on)
{
  wimp_auto_scroll_info info;

  if (wimpreadsysinfo_version() < wimp_VERSION_RO40)
    return;

  if (!on)
  {
    wimp_auto_scroll(0, NULL);
    return;
  }

  event_set_interval(10);

  info.w                   = sl->w;
  info.pause_zone_sizes.x0 = 0;
  info.pause_zone_sizes.y0 = sl->height / 2;
  info.pause_zone_sizes.x1 = 0;
  info.pause_zone_sizes.y1 = sl->height / 2;
  info.pause_duration      = 0; /* no pause */
  info.state_change        = wimp_AUTO_SCROLL_NO_HANDLER; /* using own pointer */
  /* info.handle           = 0; N/A */

  wimp_auto_scroll(wimp_AUTO_SCROLL_ENABLE_VERTICAL, &info);
}
