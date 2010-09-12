/* --------------------------------------------------------------------------
 *    Name: image-observer.h
 * Purpose: Informs clients when image objects change
 * Version: $Id: image-observer.h,v 1.1 2009-04-28 23:32:23 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_IMAGE_OBSERVER_H
#define APPENGINE_IMAGE_OBSERVER_H

#include "appengine/graphics/image.h"

typedef int imageobserver_change;

enum
{
  imageobserver_CHANGE_ABOUT_TO_DESTROY,

  imageobserver_CHANGE_PREVIEW,

  imageobserver_CHANGE_ABOUT_TO_MODIFY,
  imageobserver_CHANGE_MODIFIED,

  imageobserver_CHANGE_HIDDEN,
  imageobserver_CHANGE_REVEALED,

  imageobserver_CHANGE_GAINED_FOCUS,
  imageobserver_CHANGE_LOST_FOCUS,
};

typedef union imageobserver_data
{
  struct
  {
    image_modified_flags flags;
  }
  modified;
}
imageobserver_data;

typedef void (imageobserver_callback)(image                *image,
                                      imageobserver_change  change,
                                      imageobserver_data   *data);

int imageobserver_register(image                  *image,
                           imageobserver_callback *callback);

int imageobserver_deregister(image                  *image,
                             imageobserver_callback *callback);

/* 'Greedy' functions are called for changes on all images. */
int imageobserver_register_greedy(imageobserver_callback *callback);

int imageobserver_deregister_greedy(imageobserver_callback *callback);

int imageobserver_event(image                *image,
                        imageobserver_change  change,
                        imageobserver_data   *data);

#endif /* APPENGINE_IMAGE_OBSERVER_H */
