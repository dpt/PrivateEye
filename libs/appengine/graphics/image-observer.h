/* --------------------------------------------------------------------------
 *    Name: image-observer.h
 * Purpose: Informs clients when image objects change
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

  imageobserver_CHANGE_SAVED,            /**< When image safely saved to disc */

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

typedef void (imageobserver_callback)(image_t              *image,
                                      imageobserver_change  change,
                                      imageobserver_data   *data,
                                      void                 *opaque);

int imageobserver_register(image_t                *image,
                           imageobserver_callback *callback,
                           void                   *opaque);

int imageobserver_deregister(image_t                *image,
                             imageobserver_callback *callback,
                             void                   *opaque);

/* 'Greedy' functions are called for changes on all images. */
int imageobserver_register_greedy(imageobserver_callback *callback,
                                  void                   *opaque);

int imageobserver_deregister_greedy(imageobserver_callback *callback,
                                    void                   *opaque);

int imageobserver_event(image_t              *image,
                        imageobserver_change  change,
                        imageobserver_data   *data);

#endif /* APPENGINE_IMAGE_OBSERVER_H */
