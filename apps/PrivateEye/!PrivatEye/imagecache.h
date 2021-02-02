/* --------------------------------------------------------------------------
 *    Name: imagecache.h
 * Purpose: Image cache
 * ----------------------------------------------------------------------- */

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

result_t imagecache_get(const char *file_name,
                        bits        load,
                        bits        exec,
                        image_t   **image);
void imagecache_empty(void);
int imagecache_get_count(void);
void imagecache_destroy(image_t *i);

#endif /* IMAGECACHE_H */
