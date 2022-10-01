/* --------------------------------------------------------------------------
 *    Name: stage.h
 * Purpose: Generates a document background
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_GRAPHICS_STAGE_H
#define APPENGINE_GRAPHICS_STAGE_H

#include "oslib/os.h"

typedef unsigned int stage_flags_t;
#define stage_FLAG_SHADOW (1u << 0)
#define stage_FLAG_PAGE   (1u << 1)

typedef struct stagebox
{
  os_colour colour;
  os_box    box;
}
stagebox_t;

typedef struct stageconfig
{
  stage_flags_t flags;
  struct
  {
    os_colour   pasteboard;
    os_colour   stroke;
    os_colour   margin;
    os_colour   page;
    os_colour   shadow;
  }
  colours;
  /* Constant/fixed dimensions (OS units) */
  int           pasteboard_min;
  int           stroke;
  int           margin;
  int           shadow;
}
stageconfig_t;

void stage_config_init(stageconfig_t *config);

/**
 * Given work area and page dimensions emits an array of boxes to draw.
 */
void stage_generate(const stageconfig_t *config,
                    int                  workarea_width,
                    int                  workarea_height,
                    int                  page_width,
                    int                  page_height,
                    int                 *min_workarea_width,
                    int                 *min_workarea_height,
                    stagebox_t          *boxes,
                    size_t              *nboxes);

#endif /* APPENGINE_GRAPHICS_STAGE_H */
