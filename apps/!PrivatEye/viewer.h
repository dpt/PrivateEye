/* --------------------------------------------------------------------------
 *    Name: viewer.h
 * Purpose: Viewer block handling header
 * Version: $Id: viewer.h,v 1.45 2009-11-29 23:18:37 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef VIEWER_H
#define VIEWER_H

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/app/choices.h"
#include "appengine/graphics/drawable.h"
#include "appengine/base/errors.h"
#include "appengine/datastruct/list.h"
#include "appengine/wimp/window.h"

#include "zones.h"

typedef struct viewer viewer;

struct viewer
{
  list_t              list;

  wimp_w              main_w; /* The "primary key" */

  WindowCapture       capture;

  int                 xfer_ref;           /* data transfer protocol ref. */

  struct
  {
    int               cur, prev;
  }
  scale;

#ifdef EYE_ZONES
  zones              *zones;
#endif

  struct
  {
    int               count;
    int               dest_x, dest_y, dest_scale;
    int               pos_x, pos_y, pos_scale; /* 16.16 fxp */
  }
  scrolling;

  struct
  {
    os_colour          colour;
    osspriteop_header *header; /* used when plotting sprites */
    void             (*prepare)(viewer *);
    void             (*draw)(wimp_draw *, viewer *, int x, int y);
  }
  background;

  image              *image;
  drawable           *drawable;

  os_box              extent;
  os_box              imgbox;

  int                 x,y;
};

/* ----------------------------------------------------------------------- */

error viewer_create(viewer **viewer);
void viewer_destroy(viewer *viewer);
viewer *viewer_find(wimp_w window_handle);
viewer *viewer_find_by_attrs(const char *file_name, bits load, bits exec);
int viewer_count_clones(viewer *viewer);

typedef int (viewer_map_callback)(viewer *, void *arg);

void viewer_map(viewer_map_callback *fn, void *arg);
void viewer_map_for_image(image *image, viewer_map_callback *fn, void *arg);

int viewer_get_count(void);
void viewer_set_extent_from_box(viewer *viewer, const os_box *box);

enum
{
  viewer_UPDATE_COLOURS = 1 << 0,
  viewer_UPDATE_SCALING = 1 << 1,
  viewer_UPDATE_EXTENT  = 1 << 2,
  viewer_UPDATE_REDRAW  = 1 << 3,
  viewer_UPDATE_FORMAT  = 1 << 4,

  viewer_UPDATE_ALL     = viewer_UPDATE_COLOURS |
                          viewer_UPDATE_SCALING |
                          viewer_UPDATE_EXTENT  |
                          viewer_UPDATE_REDRAW  |
                          viewer_UPDATE_FORMAT
};

typedef unsigned int viewer_update_flags;

void viewer_update(viewer *viewer, viewer_update_flags flags);
void viewer_update_all(viewer_update_flags flags);

void viewer_open(viewer *viewer);

osbool viewer_load(viewer     *viewer,
                   const char *file_name,
                   bits        load,
                   bits        exec);
void viewer_unload(viewer *viewer);

int viewer_query_unload(viewer *viewer);

osbool viewer_save(viewer *viewer, const char *file_name);

void viewer_clone(viewer *viewer);
error viewer_clone_from_window(wimp_w w, viewer **new_viewer);

void viewer_close_all(void);

int viewer_count_edited(void);

error viewer_choices_updated(const choices_group *g);

#endif /* VIEWER_H */
