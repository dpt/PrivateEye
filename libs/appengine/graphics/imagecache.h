/* --------------------------------------------------------------------------
 *    Name: imagecache.h
 * Purpose: Read-through image cache
 * ----------------------------------------------------------------------- */

/* This is a read-through image cache. It will hold any number of active
 * images (those with +ve refcounts) and up to <maxidle> bytes of images with
 * zero refcounts. Eviction is LRU. */

#ifndef APPENGINE_IMAGECACHE_H
#define APPENGINE_IMAGECACHE_H

#include <stddef.h>

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

typedef struct imagecache imagecache_t;

/* Create an image cache that will allow 'maxidle' bytes to idle in store and
 * will store up to 'maxentries' entries. */
result_t imagecache_create(size_t         maxidle,
                           size_t         maxentries,
                           imagecache_t **newcache);
void imagecache_destroy(imagecache_t *cache);

result_t imagecache_resize(imagecache_t *cache, size_t maxidle);

/* Get an image from the cache. */
result_t imagecache_get(imagecache_t  *cache,
                  const image_choices *choices,
                  const char          *file_name,
                        bits           load,
                        bits           exec,
                        image_t      **image);

/* Dispose of an image. */
result_t imagecache_dispose(imagecache_t *cache,
                            image_t      *image);

/* Empty the cache. */
void imagecache_empty(imagecache_t *cache);

int imagecache_get_count(imagecache_t *cache);

#endif /* APPENGINE_IMAGECACHE_H */
