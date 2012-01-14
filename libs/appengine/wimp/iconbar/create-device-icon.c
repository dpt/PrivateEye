
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "oslib/os.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/base/errors.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"

#include "appengine/wimp/iconbar.h"

// I DON'T THINK THIS CODE HAS EVER BEEN EXERCISED

wimp_i iconbar_create_device_icon(const char   *sprite,
                                  const char   *text,
                                  wimp_w        w,
                                  wimp_priority priority)
{
  int               width, height;
  os_mode           mode;
  int               xeig, yeig;
  int               text_length;
  char             *validation;
  wimp_icon_create  create;

  if (EC(xwimpspriteop_read_sprite_info(sprite,
                                       &width, &height,
                                        NULL,
                                       &mode)))
  {
    /* FIXME: should check error number really */
    width  = 34;
    height = 34;
    mode   = os_MODE4BPP90X90;
  }

  os_read_mode_variable(mode, os_MODEVAR_XEIG_FACTOR, &xeig);
  os_read_mode_variable(mode, os_MODEVAR_YEIG_FACTOR, &yeig);

  text_length = strlen(text);

  validation = malloc(1 + strlen(sprite) + 1);
  /* hopefully a bit less costly than a sprintf */
  *validation = 'S';
  strcpy(validation + 1, sprite);

  /* Wimp 3.20 or later will auto-size the iconbar icon for us if the sprite
   * and text's x-size are the same */

  create.w = w;
  create.icon.extent.x0 = 0;
  create.icon.extent.y0 = -16;
  create.icon.extent.x1 = width  << xeig;
  create.icon.extent.y1 = 20 + (height << yeig);
  create.icon.flags     = wimp_ICON_TEXT     |
                          wimp_ICON_SPRITE   |
                          wimp_ICON_HCENTRED |
                          wimp_ICON_VCENTRED |
                         (wimp_BUTTON_CLICK           << wimp_ICON_BUTTON_TYPE_SHIFT) |
                         (wimp_COLOUR_BLACK           << wimp_ICON_FG_COLOUR_SHIFT) |
                         (wimp_COLOUR_VERY_LIGHT_GREY << wimp_ICON_BG_COLOUR_SHIFT);
  create.icon.data.indirected_text.text       = str_dup(text);
  if (create.icon.data.indirected_text.text == NULL)
    goto oom;

  create.icon.data.indirected_text.validation = validation;
  create.icon.data.indirected_text.size       = text_length + 1;
  strncpy(create.icon.data.sprite, sprite, 12);

  return wimp_create_icon_prioritised(priority, &create);


oom:

  error_fatal_oom();

  return 0; /* never reached */
}
