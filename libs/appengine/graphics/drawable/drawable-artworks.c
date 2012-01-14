/* --------------------------------------------------------------------------
 *    Name: artworks.c
 * Purpose: ArtWorks module
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/drawfile.h"

#include "appengine/graphics/artworks.h"
#include "appengine/graphics/awrender.h"

#include "drawable-vector.h"

#include "drawable-artworks.h"

static void artworks_redraw(const drawable_choices *choices,
                            wimp_draw              *draw,
                            drawable_t             *drawable,
                            int                     x,
                            int                     y)
{
  os_trfm            *trfm;
  awrender_info_block info_block;
  int                 scale;
  int                 mul;
  artworks_handle     handle;

  info_block.dither_x = x;
  info_block.dither_y = y;

  /* Retrieve scale value from the transform set up in
   * vector_scaling. */
  trfm = &drawable->details.generic.trfm;
#define SCALE_100PC 100
  scale = (trfm->entries[0][0] * SCALE_100PC) >> 16;
  mul = draw_OS_UNIT * SCALE_100PC;

  info_block.clip_rect.x0 = (draw->clip.x0 - x) * mul / scale;
  info_block.clip_rect.y0 = (draw->clip.y0 - y) * mul / scale;
  info_block.clip_rect.x1 = (draw->clip.x1 - x) * mul / scale;
  info_block.clip_rect.y1 = (draw->clip.y1 - y) * mul / scale;

  trfm = &drawable->details.artworks.trfm;

  trfm->entries[2][0] = x * draw_OS_UNIT;
  trfm->entries[2][1] = y * draw_OS_UNIT;

  handle.resizable_block = &drawable->image->details.artworks.workspace;
  handle.fixed_block     = &drawable->image->image;

  /* Shouldn't report errors in redraw loops. */
  (void)    awrender_render(drawable->image->image,
                           &info_block,
                            trfm,
    (awrender_vdu_block *) &drawable->details.artworks.vdu_block,
                            drawable->image->details.artworks.workspace,
(awrender_callback_handler) artworks_callback,
                            choices->artworks.quality,
                            awrender_OutputToVDU,
                           &handle);
}

static void artworks_colours(drawable_t *drawable)
{
  static const os_VDU_VAR_LIST(4) var_list =
  {{
    os_MODEVAR_XEIG_FACTOR,
    os_MODEVAR_YEIG_FACTOR,
    os_MODEVAR_LOG2_BPP,
    -1,
  }};

  os_read_vdu_variables((const os_vdu_var_list *) &var_list,
                        (int *) &drawable->details.artworks.vdu_block);

  wimp_read_true_palette(drawable->details.artworks.vdu_block.palette);
}

void drawableartworks_export_methods(drawable_t *drawable)
{
  static const drawable_methods methods =
  {
    artworks_redraw,
    artworks_colours,
    vector_scaling,
    vector_get_dimensions,
    NULL, /* reset */
    NULL
  };

  drawable->methods = methods;

  drawable->flags |= drawable_FLAG_DRAW_BG;
}
