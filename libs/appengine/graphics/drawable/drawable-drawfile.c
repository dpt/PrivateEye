/* --------------------------------------------------------------------------
 *    Name: drawfile.c
 * Purpose: DrawFile module
 * Version: $Id: drawable-drawfile.c,v 1.2 2009-09-13 17:02:13 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/drawfile.h"

#include "drawable-vector.h"

#include "drawable-drawfile.h"

static void drawfile_redraw(const drawable_choices *choices, wimp_draw *draw, drawable_t *drawable, int x, int y)
{
  int                     flatness;
  const drawfile_diagram *diagram;
  os_trfm                *trfm;

  diagram = drawable->image->image;

  trfm = &drawable->details.drawfile.trfm;

  trfm->entries[2][0] = x * draw_OS_UNIT;
  trfm->entries[2][1] = y * draw_OS_UNIT;

  flatness = choices->drawfile.flatness;

  (void) xdrawfile_render((flatness == -1) ? 0 : drawfile_RENDER_GIVEN_FLATNESS,
                          diagram,
                          drawable->image->display.file_size,
                          trfm,
                         &draw->clip,
                          flatness);
}

void drawabledrawfile_export_methods(drawable_t *drawable)
{
  static const drawable_methods drawable_drawfile_methods =
  {
    drawfile_redraw,
    NULL,
    vector_scaling,
    vector_get_dimensions,
    NULL,
    NULL
  };

  drawable->methods = drawable_drawfile_methods;

  drawable->flags |= drawable_FLAG_DRAW_BG;
}

