/* --------------------------------------------------------------------------
 *    Name: info.c
 * Purpose: Retrieves sprite dimensions, mode and log2bpp
 * ----------------------------------------------------------------------- */

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/osspriteop.h"

#include "appengine/vdu/sprite.h"

void sprite_info(const osspriteop_area   *area,
                 const osspriteop_header *header,
                 int                     *width,
                 int                     *height,
                 osbool                  *mask,
                 os_mode                 *mode,
                 int                     *log2bpp)
{
  osspriteop_read_sprite_info(osspriteop_PTR,
                              area,
              (osspriteop_id) header,
                              width,
                              height,
                              mask,
                              mode);

  if (log2bpp)
    os_read_mode_variable(*mode, os_MODEVAR_LOG2_BPP, log2bpp);
}

int sprite_type(osspriteop_mode_word mode_word)
{
  if ((mode_word & osspriteop_EXT_STYLE) == osspriteop_EXT_STYLE)
    return (mode_word & osspriteop_EXT_TYPE) >> osspriteop_EXT_TYPE_SHIFT;
  else if ((mode_word & osspriteop_NEW_STYLE) == osspriteop_NEW_STYLE)
    return (mode_word & osspriteop_TYPE) >> osspriteop_TYPE_SHIFT;
  else
    return osspriteop_TYPE_OLD;
}
