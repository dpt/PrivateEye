/* --------------------------------------------------------------------------
 *    Name: imagecache.c
 * Purpose: Image cache
 * ----------------------------------------------------------------------- */

// caches all images requested
// disposed/non-open images are subject to a size limit and eviction

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

/* ----------------------------------------------------------------------- */

typedef struct entry
{
  image_t *image;
}
entry_t;

struct imagecache
{
  size_t  maxsize;          /* max 'idle' bytes */
  int     nentries;         /* number of used entries */
  int     maxentries;
  entry_t entries[UNKNOWN]; /* stored in age order: oldest come first */
};

/* ----------------------------------------------------------------------- */

result_t imagecache_create(size_t         maxsize,
                           size_t         maxentries,
                           imagecache_t **newcache)
{
  imagecache_t *cache;

  *newcache = NULL;

  cache = malloc(offsetof(imagecache_t, entries) + maxentries * sizeof(cache->entries[0]));
  if (cache == NULL)
    return result_OOM;

  cache->maxsize    = maxsize;
  cache->nentries   = 0;
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

/* ----------------------------------------------------------------------- */

/* Return the total number of evictable bytes. */
static int evictable_bytes(const imagecache_t *cache)
{
  size_t total;
  int    i;

  total = 0;
  for (i = 0; i < cache->nentries; i++)
    if (cache->entries[i].image->refcount == 0)
      total += cache->entries[i].image->display.file_size;
  return total;
}

/* Evict the oldest evictable entry to make space. */
static int evict_oldest_image(imagecache_t *cache)
{
  int i;

  /* Proceed oldest..newest */
  for (i = 0; i < cache->nentries; i++)
    if (cache->entries[i].image->refcount == 0)
      break;

  if (i == cache->nentries)
    return 1; /* failed to find an evictable entry */

  image_destroy(cache->entries[i].image);

  array_delete_element(cache->entries,
                       sizeof(cache->entries[0]),
                       cache->nentries,
                       i);
  cache->nentries--;

  return 0; /* evicted one entry */
}

/* Evicts enough entries to satisfy 'need' bytes. */
static int evict_nbytes(imagecache_t *cache, size_t need)
{
  size_t total;
  int    i;

  /* sum the size of the oldest entries until a sufficient amount is
   * achieved */

  total = 0;
  for (i = 0; i < cache->nentries; i++)
  {
    image_t *image = cache->entries[i].image;

    if (image->refcount > 0)
      continue;

    total += image->display.file_size;

    image_destroy(image);
   
    array_delete_element(cache->entries,
                         sizeof(cache->entries[0]),
                         cache->nentries,
                         i);
    cache->nentries--;
    
    if (total >= need)
      return 0;
  }

  if (total < need)
    return 1; /* didn't find enough space */

  return 0;
}

/* ----------------------------------------------------------------------- */

result_t imagecache_get(imagecache_t  *cache,
                  const image_choices *choices,
                  const char          *file_name,
                        bits           load,
                        bits           exec,
                        image_t      **image)
{
  result_t rc;
  int      i;
  image_t *cached_image = NULL;
  image_t *new_image    = NULL;

  *image = NULL;

  /* is the file already in our cache? */
  for (i = 0; i < cache->nentries; i++)
  {
    cached_image = cache->entries[i].image;
    if (cached_image &&
        strcmp(cached_image->file_name, file_name) == 0 &&
        cached_image->source.load == load &&
        cached_image->source.exec == exec)
      break;
  }

  if (i < cache->nentries)
  {
    /* yes, it's in the cache */

    /* make cached image the youngest */
    array_delete_element(cache->entries,
                         sizeof(cache->entries[0]),
                         cache->nentries,
                         i);
    cache->entries[cache->nentries - 1].image = cached_image;
  
    image_addref(cached_image);

    /* tell any observers that we're revealing the image */
    // perhaps only when refs goes 0 -> 1 ?
    image_reveal(cached_image);

    *image = cached_image;

    return result_OK;
  }

  /* no, it's not in the cache - create it and insert */

  for (;;)
  {
    /* this is a bit of a headbanger - will repeatedly evict and retry */

    rc = image_create_from_file(choices, file_name, (load >> 8) & 0xfff, &new_image);
    if (rc)
    {
      if (rc != result_OOM)
        return rc; /* it's a proper error so bail */

      if (cache->nentries == 0)
        return result_OOM; /* nothing left to discard - return OOM */

      /* evict and retry */
      evict_oldest_image(cache);
    }
    else
    {
      assert(cache->nentries < cache->maxentries); // TODO redo properly

      /* insert at the end (youngest entry) */
      cache->entries[cache->nentries].image = new_image;
      cache->nentries++;

      *image = new_image;

      return result_OK;
    }
  }
}

// an image (which already sits in the imagecache entries) is to be disposed
// of. decide whether to destroy it, or let it sit in the cache.
result_t imagecache_dispose(imagecache_t *cache, image_t *image)
{
  int    i;
  size_t need;
  size_t free;
  
  for (i = 0; i < cache->nentries; i++)
    if (cache->entries[i].image == image)
      break;

  assert(i != cache->nentries);

  /* tell any observers that we're hiding the image */
  image_hide(image);

  image_deleteref(image);

  /* don't retain modified images */
  if (image->flags & image_FLAG_MODIFIED)
    goto destroy;

  // if (image->refcount == 0) { ...

  /* Don't retain the image if the size is larger than the cache. That would
   * just cause all entries to be evicted in the attempt to find enough
   * space. Finally it would fail anyway.
   */
  need = image->display.file_size;
  if (need > cache->maxsize)
    goto destroy;

  // replace this with evict_until_nbytes_free() ?
  free = cache->maxsize - evictable_bytes(cache);
  if (need > free) /* enough free space? */
    (void) evict_nbytes(cache, need - free); /* no - need to evict */
  // if < 1 then destroy

  /* there's enough free space now */

  /* make cached image the youngest */
  array_delete_element(cache->entries,
                       sizeof(cache->entries[0]),
                       cache->nentries,
                       i);
  cache->entries[cache->nentries - 1].image = image;

  return result_OK; /* retained in the cache */


destroy:
  array_delete_element(cache->entries,
                       sizeof(cache->entries[0]),
                       cache->nentries,
                       i);
  cache->nentries--;

  image_destroy(image);

  return result_OK; /* not retained in the cache */
}

/* ----------------------------------------------------------------------- */

void imagecache_empty(imagecache_t *cache)
{
  int i;

  hourglass_on();

  /* It's an LRU cache with oldest entries stored first. If we free in
   * reverse order we may speed up the emptying process by avoiding the need
   * to repeatedly shift the flex heap down. */

  for (i = cache->nentries - 1; i >= 0; i--)
    if (cache->entries[i].image->refcount == 0)
    {
      image_destroy(cache->entries[i].image);

      array_delete_element(cache->entries,
                           sizeof(cache->entries[0]),
                           cache->nentries,
                           i);
      cache->nentries--;
    }

  hourglass_off();
}

int imagecache_get_count(imagecache_t *cache)
{
  return cache->nentries;
}

