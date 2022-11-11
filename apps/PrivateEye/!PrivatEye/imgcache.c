/* --------------------------------------------------------------------------
 *    Name: imgcache.c
 * Purpose: Image cache
 * ----------------------------------------------------------------------- */

#include "appengine/base/errors.h"
#include "appengine/graphics/imagecache.h"

#include "globals.h"

#include "imgcache.h"

static int imgcache_inited = 0;

result_t imgcache_init(void)
{
  result_t rc;

  rc = imagecache_create(GLOBALS.choices.cache.size * 1024,
                         GLOBALS.choices.cache.entries,
                        &GLOBALS.cache);
  if (rc == result_OK)
    imgcache_inited = 1;

  return rc;
}

void imgcache_fin(void)
{
  if (imgcache_inited)
    imagecache_destroy(GLOBALS.cache);
}

result_t imgcache_choices_updated(const choices_group *group)
{
  result_t rc = result_OK;

  NOT_USED(group);

  if (imgcache_inited)
    rc = imagecache_resize(GLOBALS.cache, GLOBALS.choices.cache.size * 1024);

  return rc;
}
