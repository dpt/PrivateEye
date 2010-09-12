/* --------------------------------------------------------------------------
 *    Name: imagecache.h
 * Purpose: Image cache
 * Version: $Id: imagecache.h,v 1.5 2009-05-20 21:38:18 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/graphics/image.h"

error imagecache_get(const char *file_name,
                     bits        load,
                     bits        exec,
                     image     **image);
void imagecache_empty(void);
int imagecache_get_count(void);
void imagecache_destroy(image *i);

#endif /* IMAGECACHE_H */
