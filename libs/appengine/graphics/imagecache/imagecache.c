/* --------------------------------------------------------------------------
 *    Name: imagecache.c
 * Purpose: Read-through image cache
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
#include <stdio.h>

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
  size_t  maxidle;          /* max 'idle' bytes */
  int     nentries;         /* number of used entries */
  int     maxentries;       /* max entries */
  entry_t entries[UNKNOWN]; /* stored in age order: oldest come first */
};

/* ----------------------------------------------------------------------- */

result_t imagecache_create(size_t         maxidle,
                           size_t         maxentries,
                           imagecache_t **newcache)
{
  imagecache_t *cache;

  *newcache = NULL;

  cache = malloc(offsetof(imagecache_t, entries) + maxentries * sizeof(cache->entries[0]));
  if (cache == NULL)
    return result_OOM;

  cache->maxidle    = maxidle;
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
  assert(cache->nentries == 0);
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

  assert(cache->entries[i].image->refcount == 0);

  image_destroy(cache->entries[i].image);

  array_delete_element(cache->entries,
                       sizeof(cache->entries[0]),
                       cache->nentries,
                       i);
  cache->nentries--;
  assert(cache->nentries >= 0);
  assert(cache->nentries <= cache->maxentries);

  return 0; /* evicted one entry */
}

/* Evicts enough entries to satisfy 'need' bytes. */
static int evict_nbytes(imagecache_t *cache, size_t need)
{
  size_t   total;
  int      i;
  image_t *image;

  total = 0;
  for (i = 0; i < cache->nentries; i++)
  {
    image = cache->entries[i].image;

    if (image->refcount > 0)
      continue;

    total += image->display.file_size;

    image_destroy(image);
 
    array_delete_element(cache->entries,
                         sizeof(cache->entries[0]),
                         cache->nentries,
                         i);
    cache->nentries--;
    i--; /* restart at the same, just overwritten, entry */

    if (total >= need)
      return 0;
  }

  return 1; /* didn't find enough space */
}

/* ----------------------------------------------------------------------- */

result_t imagecache_resize(imagecache_t *cache, size_t newmaxidle)
{
  if (newmaxidle < cache->maxidle)
  {
    size_t reduction;

    reduction = cache->maxidle - newmaxidle;
    if (reduction)
      (void) evict_nbytes(cache, reduction);
  }

  cache->maxidle = newmaxidle;

  return result_OK;
}

/* ----------------------------------------------------------------------- */

/* Make the i'th cached image the youngest. */
static void make_youngest(imagecache_t *cache, int i)
{
  image_t *image;

  if (i == cache->nentries - 1)
    return; /* already youngest */

  image = cache->entries[i].image;
  array_delete_element(cache->entries,
                       sizeof(cache->entries[0]),
                       cache->nentries,
                       i);
  cache->entries[cache->nentries - 1].image = image;
}

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

  /* Is the file already in our cache? */
  for (i = 0; i < cache->nentries; i++)
  {
    cached_image = cache->entries[i].image;
    assert(cached_image);
    if (strcmp(cached_image->file_name, file_name) == 0 &&
        cached_image->source.load == load &&
        cached_image->source.exec == exec)
      break;
  }

  if (i < cache->nentries)
  {
    /* Yes, it's in the cache. */

    make_youngest(cache, i);
    image_addref(cached_image);

    /* Tell observers when the cached image becomes active. */
    if (cached_image->refcount == 1)
      image_reveal(cached_image);

    *image = cached_image;

    return result_OK;
  }

  /* No, it's not in the cache - create it and insert it.
   *
   * Unfortunately this is a bit of a headbanger since we don't know how much
   * memory the image will need in advance. We repeatedly try to create the
   * image and while we're seeing OOM we will evict and retry until either it
   * works, or the cache is emptied and we still OOM, so give up. */
  for (;;)
  {
    rc = image_create_from_file(choices,
                                file_name,
                               (load >> 8) & 0xfff,
                               &new_image);
    if (rc)
    {
      if (rc != result_OOM)
        return rc; /* a proper error: bail */

      if (cache->nentries == 0)
        return result_OOM; /* nothing left to discard - return OOM */

      /* Evict and retry. */
      evict_oldest_image(cache);
    }
    else
    {
      if (cache->nentries == cache->maxentries)
        return result_OOM; /* out of entries */

      /* Insert at the end (the youngest entry). */
      cache->entries[cache->nentries].image = new_image;
      cache->nentries++;

      *image = new_image;

      return result_OK;
    }
  }
}

/* An image which already sits in the image cache entries is to be disposed
 * of. Decide whether to destroy it or to let it continue to sit in the
 * cache. */
result_t imagecache_dispose(imagecache_t *cache, image_t *image)
{
  int    i;
  size_t need;

  if (image == NULL)
    return result_OK;
  
  for (i = 0; i < cache->nentries; i++)
    if (cache->entries[i].image == image)
      break;

  if (i == cache->nentries)
    return result_NOT_FOUND; /* an unknown image; not in the cache */

  need = image->display.file_size;
  if ((image->flags & image_FLAG_MODIFIED) || (need > cache->maxidle))
  {
    int refcount;

    /* Don't retain modified images (shouldn't be cached) or those too large
     * to fit in the cache (would just cause all other entries to be
     * evicted). */
    
    /* image_destroy() doesn't really destroy until the refcount hits zero,
     * so the image may or may not have gone away at this point. If it's got
     * no references left then we remove it from the cache. */
    refcount = image->refcount;
    if (refcount == 1)
      image_hide(image);
    image_destroy(image);
    if (refcount == 1) /* the image went away */
    {
      array_delete_element(cache->entries,
                           sizeof(cache->entries[0]),
                           cache->nentries,
                           i);
      cache->nentries--;
      assert(cache->nentries >= 0);
      assert(cache->nentries <= cache->maxentries);
    }
  }
  else
  {
    /* This image is eligible to be retained in the cache, which it's already
     * sitting in anyway. */

    if (image->refcount == 1)
    {
      size_t free;

      /* This image will change from active (refcount>0) to cached
       * (refcount=0). Should we keep it? */

      free = cache->maxidle - evictable_bytes(cache);
      if (need > free) /* enough free space? */
        (void) evict_nbytes(cache, need - free); /* not enough - need to evict */
   
      /* There's enough free space now.
       * Tell any observers that we're hiding the image. */
      image_hide(image);
      image_deleteref(image); /* zero refcount without triggering destroy */
      assert(image->refcount == 0);

      make_youngest(cache, i);
    }
    else
    {
      image_deleteref(image); /* decrement refcount... will zero refcount without triggering destroy */
    }
  }

  return result_OK;
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
      assert(cache->nentries >= 0);
      assert(cache->nentries <= cache->maxentries);
    }

  hourglass_off();
}

int imagecache_get_count(imagecache_t *cache)
{
  return cache->nentries;
}

