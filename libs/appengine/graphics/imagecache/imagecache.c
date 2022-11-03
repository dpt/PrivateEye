/* --------------------------------------------------------------------------
 *    Name: imagecache.c
 * Purpose: Image cache
 * ----------------------------------------------------------------------- */

/* TODO
 *
 * - What should we do about changes to choices, e.g. sprite->jpeg
 *   conversion?
 *   For example: With the cache on, load a JPEG as a JPEG. Change the load
 *   options to convert to Sprite. Load the JPEG again. The cache will pick
 *   up the non-Sprite image.
 * - Include load and exec stamps in the cache keys.
 */

#include <assert.h>
#include <string.h>

#include "oslib/hourglass.h"

#include "appengine/datastruct/array.h"
#include "appengine/graphics/image.h"

#include "appengine/graphics/imagecache.h"

typedef struct entry
{
  image_t *image;
}
entry_t;

struct imagecache
{
  size_t  used;     /* in bytes */
  size_t  maxsize;  /* in bytes */
  int     nentries; /* number of used entries */
  int     maxentries;
  entry_t entries[UNKNOWN];  /* stored in age order: oldest come first */
};

result_t imagecache_create(size_t         maxsize,
                           size_t         maxentries,
                           imagecache_t **newcache)
{
  imagecache_t *cache;

  *newcache = NULL;

  cache = calloc(offsetof(imagecache_t, entries) + maxentries * sizeof(cache->entries[0]), 1);
  if (cache == NULL)
    return result_OOM;

  cache->maxsize    = maxsize;
  cache->maxentries = maxentries;

  *newcache = cache;

  return result_OK;
}

void imagecache_destroy(imagecache_t *cache)
{
  if (cache == NULL)
    return;
  
  imagecache_empty(cache);
  free(cache);
}

result_t imagecache_get(imagecache_t *cache,
                        const char   *file_name,
                        bits          load,
                        bits          exec,
                        image_t     **image)
{
  int      i;
  image_t *ci = NULL;

  /* is the file already in our cache? */
  for (i = 0; i < cache->nentries; i++)
  {
    ci = cache->entries[i].image;
    if (ci &&
        strcmp(ci->file_name, file_name) == 0 &&
        ci->source.load == load &&
        ci->source.exec == exec)
      break;
  }

  if (i == cache->nentries)
  {
    /* no, it's not in the cache */

    // TODO: Make it!

    *image = NULL;
  }
  else
  {
    /* yes, it's in the cache */

    array_delete_element(cache->entries,
                         sizeof(cache->entries[0]),
                         cache->nentries,
                         i);

    cache->nentries -= 1;
    cache->used     -= ci->display.file_size;

    image_addref(ci);

    /* tell any observers that we're revealing the image */
    image_reveal(ci);

    *image = ci;
  }

  return result_OK;
}

static int evict_one_image(imagecache_t *cache)
{
  int i;
  int t;

  i = 0; /* oldest entry */

  t = cache->entries[i].image->display.file_size;

  /* evict the oldest entry to make space */

  image_destroy(cache->entries[i].image);

  array_delete_element(cache->entries,
                       sizeof(cache->entries[0]),
                       cache->nentries,
                       i);

  cache->nentries -= 1;
  cache->used     -= t;

  return 0;
}

/* Evicts enough entries to satisfy 'need' bytes. */
static int evict_nbytes(imagecache_t *cache, size_t need)
{
  size_t t;
  int    i;
  int    j;

  /* sum the size of the oldest entries until a sufficient amount is
   * achieved */

  t = 0;
  for (i = 0; i < cache->nentries; i++)
  {
    t += cache->entries[i].image->display.file_size;
    if (t >= need)
      break;
  }

  if (t < need)
    return 1; /* didn't find enough space */

  i++; /* make exclusive */

  /* evict the oldest entries to make space */

  for (j = 0; j < i; j++)
    image_destroy(cache->entries[j].image);

  array_delete_elements(cache->entries,
                        sizeof(cache->entries[0]),
                        cache->nentries,
                        0,
                        i - 1);

  cache->nentries -= i;
  cache->used     -= t;

  return 0;
}

result_t imagecache_dispose(imagecache_t *cache, image_t *image)
{
  size_t need;
  size_t free;

  need = image->display.file_size;

  /* Don't bother to insert if the image size is larger than the cache. That
   * would just cause all entries to be evicted in the attempt to find enough
   * space. Finally it would fail anyway.
   */
  if (need > cache->maxsize)
    return result_OK; /* not inserted */

  free = cache->maxsize - cache->used;

  if (need > free) /* enough free space? */
    evict_nbytes(cache, need - free); /* no - need to evict */

  /* there's enough free space now */

  if (cache->nentries == cache->maxentries) /* a free cache entry? */
    evict_one_image(cache); /* no - force one eviction */

  /* there's a free cache entry now */

  /* insert at the end (youngest entry) */
  cache->entries[cache->nentries].image = image;
  cache->nentries++;
  cache->used += need;

  image_deleteref(image);

  /* tell any observers that we're hiding the image */
  image_hide(image);

  return result_OK; /* inserted */
}

void imagecache_empty(imagecache_t *cache)
{
  int i;

  hourglass_on();

  /* The entries are stored in age order, so oldest comes first. If we free
   * in reverse order we may speed up the emptying process by avoiding the
   * need to repeatedly shift the heap down. */

  for (i = cache->nentries - 1; i >= 0; i--)
    image_destroy(cache->entries[i].image);

  cache->nentries = 0;
  cache->used     = 0;

  hourglass_off();
}

int imagecache_get_count(imagecache_t *cache)
{
  return cache->nentries;
}

// /* This is like image_destroy but uses the refcount to see whether to cache
//  * the image. */
// void imagecache_put(imagecache_t *cache, image_t *image)
// {
//   int inserted = 0;
// 
//   /* destroy modified images */
//   if (image->flags & image_FLAG_MODIFIED)
//     goto destroy;
// 
//   /* if there are (will be) no references left to it,
//    * then it's eligible to be cached. */
//   if (image->refcount == 1)
//     inserted = imagecache_dispose(cache, image);
// 
//   /* otherwise it might not have fit in the cache */
//   if (!inserted)
//     goto destroy;
// 
//   return;
// 
// 
// destroy:
// 
//   image_destroy(image);
// }
