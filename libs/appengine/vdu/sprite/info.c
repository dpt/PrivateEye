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
