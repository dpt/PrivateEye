/* --------------------------------------------------------------------------
 *    Name: viewer.h
 * Purpose: Viewer block handling header
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_H
#define VIEWER_H

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "datastruct/list.h"

#include "appengine/app/choices.h"
#include "appengine/graphics/drawable.h"
#include "appengine/graphics/stage.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/window.h"

#include "zones.h"

typedef struct viewer_t viewer_t;

struct viewer_t
{
  list_t              list;     /* A viewer is a linked list node. */

  wimp_w              main_w;   /* The "primary key". */

  WindowCapture       capture;  /* Captured window position. */

  int                 xfer_ref; /* Data transfer protocol ref. */

  struct
  {
    int               cur, prev; /* Current, previous scale factor. */
  }
  scale;

#ifdef EYE_ZONES
  zones              *zones;    /* Records which parts of the image have been
                                 * 'seen'. */
#endif

  struct
  {
    int               count;
    int               dest_x, dest_y, dest_scale;
    int               pos_x, pos_y, pos_scale; /* 16.16 fixed point */
  }
  scrolling;

  struct
  {
    os_colour         colour;
    void            (*draw)(wimp_draw *, viewer_t *, int x, int y);
    struct
    {
      stageconfig_t   config;
      stagebox_t      boxes[stage_MAX_BOXES];
      size_t          nboxes;
    }
    stage;
  }
  background;

  image_t            *image;    /* The image we're viewing. */
  drawable_t         *drawable; /* How to draw the image. */

  os_box              extent;   /* Bounding box of the viewer window. */
  os_box              imgdims;  /* Bounding box of the image in its space. */
  os_box              imgbox;   /* Bounding box of the image within the window. */
};

/* ----------------------------------------------------------------------- */

result_t viewer_create(viewer_t **viewer);
void viewer_destroy(viewer_t *viewer);
viewer_t *viewer_find(wimp_w window_handle);
viewer_t *viewer_find_by_attrs(const char *file_name, bits load, bits exec);
int viewer_count_clones(viewer_t *viewer);

typedef int (viewer_map_callback)(viewer_t *, void *opaque);

void viewer_map(viewer_map_callback *fn, void *opaque);
void viewer_map_for_image(image_t *image, viewer_map_callback *fn, void *opaque);

int viewer_get_count(void);
void viewer_mode_change(void);
void viewer_set_extent_from_box(viewer_t *viewer, const os_box *box);

enum
{
  viewer_UPDATE_COLOURS = 1 << 0,
  viewer_UPDATE_SCALING = 1 << 1,
  viewer_UPDATE_EXTENT  = 1 << 2,
  viewer_UPDATE_CONTENT = 1 << 3, /* just the image */
  viewer_UPDATE_REDRAW  = 1 << 4, /* the whole window */
  viewer_UPDATE_FORMAT  = 1 << 5,

  /* This doesn't include _CONTENT since that's covered by _REDRAW. */
  viewer_UPDATE_ALL     = viewer_UPDATE_COLOURS |
                          viewer_UPDATE_SCALING |
                          viewer_UPDATE_EXTENT  |
                          viewer_UPDATE_REDRAW  |
                          viewer_UPDATE_FORMAT
};

typedef unsigned int viewer_update_flags;

void viewer_update(viewer_t *viewer, viewer_update_flags flags);
void viewer_update_all(viewer_update_flags flags);

void viewer_open(viewer_t *viewer);

osbool viewer_load(viewer_t   *viewer,
                   const char *file_name,
                   bits        load,
                   bits        exec,
                   osbool      unsafe,
                   osbool      template);
void viewer_unload(viewer_t *viewer);

int viewer_query_unload(viewer_t *viewer);

osbool viewer_save(viewer_t *viewer, const char *file_name, osbool unsafe);

void viewer_clone(viewer_t *viewer);
result_t viewer_clone_from_window(wimp_w w, viewer_t **new_viewer);

void viewer_close_all(void);

int viewer_count_edited(void);

result_t viewer_choices_updated(const choices_group *g);

#endif /* VIEWER_H */
