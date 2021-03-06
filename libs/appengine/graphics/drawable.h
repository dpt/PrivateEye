/* --------------------------------------------------------------------------
 *    Name: drawable.h
 * Purpose: Drawable images
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DRAWABLE_H
#define APPENGINE_DRAWABLE_H

#include "oslib/types.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

#include "awrender.h"

#define T drawable_t

/* ----------------------------------------------------------------------- */

typedef int artworks_quality;
enum
{
  artworks_OUTLINE      = 0,
  artworks_SIMPLE       = 50,
  artworks_NORMAL       = 100,
  artworks_ANTI_ALIASED = 110
};

typedef int drawfile_flatness;
enum
{
  drawfile_FLATNESS_COARSE = 1 << 12,
  drawfile_FLATNESS_NORMAL = 1 << 8,
  drawfile_FLATNESS_BEST   = 1 << 0,
  drawfile_FLATNESS_AUTO   = 0
};

typedef struct
{
  struct
  {
    artworks_quality  quality;
  }
  artworks;

  struct
  {
    drawfile_flatness flatness;
  }
  drawfile;

  struct
  {
    jpeg_scale_flags  plot_flags;
  }
  jpeg;

  struct
  {
    osspriteop_action plot_flags;
    osbool            use_tinct;
  }
  sprite;
}
drawable_choices;

/* ----------------------------------------------------------------------- */

enum
{
  drawable_FLAG_DRAW_BG  = 1 << 0
};

typedef unsigned int drawable_flags;


typedef struct T T;


struct drawable_methods
{
  void (*redraw)  (const drawable_choices *choices,
                         wimp_draw        *draw, /* used only for clip box */
                         T                *drawable,
                         int               x,
                         int               y);

  void (*update_colours)(T                *drawable);

  void (*update_scaling)(T                *drawable,
                         const os_factors *factors);

  /* gets /scaled/ dimensions */
  void (*get_dimensions)(T                *drawable,
                         const os_factors *factors,
                         os_box           *box);

  void (*reset)         (T                *drawable);

  void (*set_index)     (T                *drawable,
                         int               index);
};

typedef struct drawable_methods drawable_methods;


struct T
{
  drawable_methods methods;

  drawable_flags   flags;

  image_t         *image;

  union
  {
    /* Note that sprite/gif/png use identical structures so that routines can
     * be used in common (i.e. sprite.trans_tab is as for png.trans_tab.) */
    struct
    {
      osspriteop_trans_tab *trans_tab;
      os_factors            factors;
      int                   index;
    }
    sprite;

    struct
    {
      os_factors            factors;
    }
    jpeg;

    /* Note that generic, drawfile and artworks start with the same members
     * for the convenience of the generic methods. */

    struct
    {
      os_trfm               trfm;
    }
    generic; /* generic to vectors */

    struct
    {
      os_trfm               trfm;
    }
    drawfile;

    struct
    {
      os_trfm               trfm;
      awrender_vdu_block    vdu_block;
    }
    artworks;
  }
  details;
};

/* ----------------------------------------------------------------------- */

result_t drawable_create(image_t *image,
                         T      **newdrawable);

result_t drawable_clone(T  *original,
                        T **newdrawable);

void drawable_destroy(T *d);

void drawable_reset(T *d);

#undef T

#endif /* APPENGINE_DRAWABLE_H */
