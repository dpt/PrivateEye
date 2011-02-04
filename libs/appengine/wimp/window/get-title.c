
#include "oslib/wimp.h"

#include "appengine/wimp/window.h"

char *window_get_title_text(wimp_w w)
{
  const wimp_icon_flags want = wimp_ICON_TEXT | wimp_ICON_INDIRECTED;

  wimp_window_info info;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  /* indirected[+sprite]+text */

  if ((info.title_flags & want) == want)
    return info.title_data.indirected_text.text;

  return "";
}
