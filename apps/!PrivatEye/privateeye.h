/* --------------------------------------------------------------------------
 *    Name: privateeye.h
 * Purpose: Picture viewer core header
 * Version: $Id: privateeye.h,v 1.29 2009-04-28 23:32:37 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef PRIVATEEYE_H
#define PRIVATEEYE_H

#define APPNAME "PrivateEye"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/jpeg.h"
#include "oslib/osspriteop.h"

#include "appengine/graphics/drawable.h"
#include "appengine/graphics/image.h"

typedef int viewer_size;
enum
{
  viewersize_FIT_TO_IMAGE  = 0,
  viewersize_FIT_TO_SCREEN = 1
};

typedef int viewer_scalefac;
enum
{
  /* Positive values are reserved for actual scale values. */
  viewerscale_FIT_TO_SCREEN = -1,
  viewerscale_FIT_TO_WINDOW = -2, /* not yet used */
  viewerscale_PRESERVE      = -3
};

typedef struct
{
  struct
  {
    viewer_size       size;
    viewer_scalefac   scale;
    int               steps;
    int               cover_icon_bar;
    int               scroll_x, scroll_y;
  }
  viewer;

  struct
  {
    int               size;
    int               item_size;
    int               padding_size;

    int               thumbnail_w, thumbnail_h;
    int               item_w, item_h;
    int               padding_h, padding_v;
  }
  thumbview;

  struct
  {
    int               size; /* in kilobytes */
  }
  cache;

  struct
  {
    int               on;
    int               gamma;
    int               brightness;
    int               contrast;
  }
  colour;

  struct
  {
    int               display;
    int               sort;
    int               size;
    int               leading;
    int               padding;
    int               scale;
    osbool            selfirst;
  }
  tagcloud;

  struct
  {
    os_colour         background;
    osbool            load;
  }
  sprite;

  struct
  {
    os_colour         background;
    osbool            load;
  }
  jpeg;

  struct
  {
    os_colour         background;
    osbool            load;
  }
  gif;

  struct
  {
    os_colour         background;
    osbool            load;
  }
  png;

  struct
  {
    os_colour         background;
    osbool            load;
  }
  drawfile;

  struct
  {
    os_colour         background;
    osbool            load;
  }
  artworks;

  struct
  {
    int               bars;
  }
  hist;

  struct
  {
    int               padding;
  }
  info;

  struct
  {
    wimp_colour       bgcolour;
    int               wrapwidth;
    int               line_height;
  }
  metadata;

  struct
  {
    int               snap;
    int               max_thumb;
  }
  rotate;

  struct
  {
    int               step;
    int               mult;
  }
  scale;

  struct
  {
    int               curve_width;
  }
  effects;

  image_choices       image;

  drawable_choices    drawable;
}
eye_choices;

#define SCALE_100PC 100

#endif /* PRIVATEEYE_H */
