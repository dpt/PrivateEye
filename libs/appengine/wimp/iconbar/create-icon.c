
#include <stddef.h>
#include <string.h>

#include "oslib/os.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"

#include "appengine/wimp/iconbar.h"

wimp_i iconbar_create_icon(const char    *sprite,
                           wimp_w         w,
                           wimp_priority  priority)
{
  int              width, height;
  os_mode          mode;
  int              xeig, yeig;
  wimp_icon_create create;

  if (EC(xwimpspriteop_read_sprite_info(sprite, &width, &height, NULL, &mode)))
  {
    /* FIXME: should check error number really */
    width  = 34;
    height = 34;
    mode   = os_MODE4BPP90X90;
  }

  os_read_mode_variable(mode, os_MODEVAR_XEIG_FACTOR, &xeig);
  os_read_mode_variable(mode, os_MODEVAR_YEIG_FACTOR, &yeig);

  create.w = w;
  create.icon.extent.x0 = 0;
  create.icon.extent.y0 = 0;
  create.icon.extent.x1 = width  << xeig;
  create.icon.extent.y1 = height << yeig;
  create.icon.flags     = wimp_ICON_SPRITE   |
                          wimp_ICON_HCENTRED |
                          wimp_ICON_VCENTRED |
                         (wimp_BUTTON_CLICK << wimp_ICON_BUTTON_TYPE_SHIFT);
  strncpy(create.icon.data.sprite, sprite, 12);

  return wimp_create_icon_prioritised(priority, &create);
}

