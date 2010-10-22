/* --------------------------------------------------------------------------
 *    Name: colours.c
 * Purpose: Common bitmap code
 * Version: $Id: colours.c,v 1.1 2009-05-21 22:27:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "flex.h"

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/colourtrans.h"
#include "oslib/osspriteop.h"

#include "appengine/vdu/screen.h"

#include "appengine/vdu/sprite.h"

int sprite_colours(osspriteop_area      **area_anc,
                   osspriteop_header     *header,
                   osspriteop_trans_tab **trans_tab_anc)
{
  osspriteop_area *area;
  int              headeroffset;
  int              palette_size;
  os_mode          mode;
  int              log2bpp;
  int              size;

  area         = *area_anc;
  headeroffset = (char *) header - (char *) area;

  osspriteop_read_palette_size(osspriteop_PTR,
                               area,
               (osspriteop_id) header,
                              &palette_size,
                               NULL /* address of palette */,
                               NULL /* mode */ );

  mode = header->mode;
  read_mode_vars(mode, NULL, NULL, &log2bpp);

  /* Find the size of the translation table */
  if (log2bpp <= 3 && palette_size == 0)
  {
    const os_palette *palette;

    palette = get_default_palette(log2bpp);

    /* 1,2,4,8bpp unpaletted */
    size = colourtrans_generate_table(mode,
                                      palette,
                                      os_CURRENT_MODE,
                                      colourtrans_CURRENT_PALETTE,
                                      NULL,
                                      colourtrans_RETURN_WIDE_ENTRIES,
                                      NULL,
                                      NULL);
  }
  else
  {
    /* 1,2,4,8bpp paletted or 16,32bpp */
    size = colourtrans_generate_table_for_sprite(area,
                                 (osspriteop_id) header,
                                                 os_CURRENT_MODE,
                                                 colourtrans_CURRENT_PALETTE,
                                                 NULL,
                                                 colourtrans_GIVEN_SPRITE | colourtrans_RETURN_WIDE_ENTRIES,
                                                 NULL,
                                                 NULL);
  }

  if (size == 0)
  {
    *trans_tab_anc = NULL; /* none required */
    return FALSE; /* ok */
  }

  if (flex_alloc((flex_ptr) trans_tab_anc, size) == 0)
    return TRUE; /* failure */

  /* HEAP HAS MOVED! - update pointers */
  area   = *area_anc;
  header = (osspriteop_header *) ((char *) area + headeroffset);

  /* obtain the translation table */
  if (log2bpp <= 3 && palette_size == 0)
  {
    const os_palette *palette;

    palette = get_default_palette(log2bpp);

    /* 1,2,4,8bpp unpaletted */
    size = colourtrans_generate_table(mode,
                                      palette,
                                      os_CURRENT_MODE,
                                      colourtrans_CURRENT_PALETTE,
                                     *trans_tab_anc,
                                      colourtrans_RETURN_WIDE_ENTRIES,
                                      NULL,
                                      NULL);
  }
  else
  {
    /* 1,2,4,8bpp paletted or 16,32bpp */
    size = colourtrans_generate_table_for_sprite(area,
                                 (osspriteop_id) header,
                                                 os_CURRENT_MODE,
                                                 colourtrans_CURRENT_PALETTE,
                                                *trans_tab_anc,
                                                 colourtrans_GIVEN_SPRITE | colourtrans_RETURN_WIDE_ENTRIES,
                                                 NULL,
                                                 NULL);
  }

  return FALSE; /* ok */
}
