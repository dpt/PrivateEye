
#include <stddef.h>
#include <limits.h>

#include "oslib/colourpicker.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/base/oserror.h"

#include "appengine/wimp/colourpick.h"

colourpicker_d colourpick_popup(wimp_w w, wimp_i i, os_colour colour)
{
  wimp_window_state     window;
  wimp_icon_state       icon;
  colourpicker_dialogue dialogue;

  window.w = w;
  wimp_get_window_state(&window);

  icon.w = w;
  icon.i = i;
  wimp_get_icon_state(&icon);

  dialogue.flags = colourpicker_DIALOGUE_OFFERS_TRANSPARENT;
  if (colour == os_COLOUR_TRANSPARENT)
    dialogue.flags |= colourpicker_DIALOGUE_TRANSPARENT;

  dialogue.title = NULL; /* default title */

  dialogue.visible.x0 = window.visible.x0 - window.xscroll + icon.icon.extent.x1 + 2;
  dialogue.visible.y0 = INT_MIN;
  dialogue.visible.x1 = INT_MAX;
  dialogue.visible.y1 = window.visible.y1 - window.yscroll + icon.icon.extent.y1;
  dialogue.xscroll    = 0;
  dialogue.yscroll    = 0;
  dialogue.colour     = colour;
  dialogue.size       = 0; /* size of extension block */

  return colourpicker_open_dialogue(colourpicker_OPEN_TRANSIENT,
                                   &dialogue,
                                    NULL);
}
