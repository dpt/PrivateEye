/* --------------------------------------------------------------------------
 *    Name: imagecache.h
 * Purpose: Image cache
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_IMAGECACHE_H
#define APPENGINE_IMAGECACHE_H

#include <stddef.h>

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

typedef struct imagecache imagecache_t;

result_t imagecache_create(size_t         maxsize,
                           size_t         maxentries,
                           imagecache_t **newcache);
void imagecache_destroy(imagecache_t *cache);

// get an image, via the cache
result_t imagecache_get(imagecache_t *cache,
                        const char   *file_name,
                        bits          load,
                        bits          exec,
                        image_t     **image);

// image can be disposed
result_t imagecache_dispose(imagecache_t *cache,
                            image_t      *image);

void imagecache_empty(imagecache_t *cache);
int imagecache_get_count(imagecache_t *cache);

#endif /* APPENGINE_IMAGECACHE_H */
