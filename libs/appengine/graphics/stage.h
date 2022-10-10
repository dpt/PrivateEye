/* --------------------------------------------------------------------------
 *    Name: stage.h
 * Purpose: Generates a document background
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_GRAPHICS_STAGE_H
#define APPENGINE_GRAPHICS_STAGE_H

#include "oslib/os.h"

/* Maximum number of boxes we'll emit */
#define stage_MAX_BOXES (17)

typedef unsigned int stageflags_t;
#define stage_FLAG_SHADOW (1u << 0) /* enable shadow */

typedef enum stageboxkind
{
  stageboxkind_PASTEBOARD,
  stageboxkind_STROKE,
  stageboxkind_MARGIN,
  stageboxkind_CONTENT,
  stageboxkind_SHADOW,
  stageboxkind__LIMIT
}
stageboxkind_t;

typedef struct stagebox
{
  stageboxkind_t kind;
  os_box         box;
}
stagebox_t;

typedef struct stageconfig
{
  stageflags_t  flags;

  /* Constant/fixed dimensions (OS units) */
  int           pasteboard_min;
  int           stroke;
  int           margin;
  int           shadow;
}
stageconfig_t;

void stageconfig_init(stageconfig_t *config);

/**
 * Return the sum of the fixed size elements that the stage wants to present.
 */
void stage_get_fixed(const stageconfig_t *config,
                     int                 *width,
                     int                 *height);

/**
 * Given work area and page dimensions emits an array of boxes to draw.
 */
void stage_generate(const stageconfig_t *config,
                    int                  workarea_width,
                    int                  workarea_height,
                    int                  content_width,
                    int                  content_height,
                    int                 *min_workarea_width,
                    int                 *min_workarea_height,
                    stagebox_t          *boxes,
                    size_t              *nboxes);

#endif /* APPENGINE_GRAPHICS_STAGE_H */
