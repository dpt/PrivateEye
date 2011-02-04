/* --------------------------------------------------------------------------
 *    Name: set-shape.c
 * Purpose: set_pointer_shape, restore_pointer_shape
 * ----------------------------------------------------------------------- */

#include <stddef.h>

#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osspriteop.h"
#include "oslib/wimpspriteop.h"

#include "appengine/wimp/window.h"

#include "appengine/wimp/pointer.h"

void set_pointer_shape(const char *name, int x, int y)
{
  enum { ShapeNo = 2 };

  /* Not sure if a translation table is required... */

  os_error *err;
  int       shape;

  err = xosspriteop_set_pointer_shape(osspriteop_USER_AREA,
                  (osspriteop_area *) window_get_sprite_area(),
                (const osspriteop_id) name,
                                      ShapeNo,
                                      x,y,
                                      NULL,
                                      NULL);

  if (err && err->errnum == error_SPRITE_OP_DOESNT_EXIST)
  {
    /* Try the Wimp sprite pool instead */

    (void) xwimpspriteop_set_pointer_shape(name,
                                           ShapeNo,
                                           x,y,
                                           NULL,
                                           NULL);
  }

  shape = osbyte1(osbyte_SELECT_POINTER, ShapeNo, 0);
  if ((shape & 0x7F) == 0)
    osbyte1(osbyte_SELECT_POINTER, shape, 0);
}

void restore_pointer_shape(void)
{
  enum { ShapeNo = 1 };

  osbyte1(osbyte_SELECT_POINTER, ShapeNo, 0);
}
