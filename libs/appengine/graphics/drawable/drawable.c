/* --------------------------------------------------------------------------
 *    Name: drawable.c
 * Purpose: drawable images
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/os.h"
#include "oslib/osfile.h"

#include "appengine/base/bsearch.h"
#include "appengine/graphics/image.h"

#include "drawable-artworks.h"
#include "drawable-bitmap.h"
#include "drawable-drawfile.h"
#include "drawable-jpeg.h"

#include "appengine/graphics/drawable.h"

static void set_methods(drawable_t *d)
{
  static const struct
  {
    bits   file_type;
    void (*export)(drawable_t *);
  }
  map[] =
  {
    { 0xaff /* osfile_TYPE_DRAW */,   drawabledrawfile_export_methods },
    { 0xc85 /* jpeg_FILE_TYPE */,     drawablejpeg_export_methods     },
    { 0xd94 /* artworks_FILE_TYPE */, drawableartworks_export_methods },
    { 0xff9 /* osfile_TYPE_SPRITE */, drawablebitmap_export_methods   }
  };

  const size_t stride = sizeof(map[0]);
  const size_t nelems = sizeof(map) / stride;

  int i;

  i = bsearch_uint(&map[0].file_type,
                    nelems,
                    stride,
                    d->image->display.file_type);

  if (i >= 0)
    map[i].export(d);
  else
    assert(0);
}

result_t drawable_create(image_t *image, drawable_t **new_drawable)
{
  drawable_t *d;

  d = malloc(sizeof(*d));
  if (d == NULL)
    return result_OOM;

  d->flags = 0;
  d->image = image;

  set_methods(d);

  *new_drawable = d;

  return result_OK;
}

result_t drawable_clone(drawable_t *original, drawable_t **new_drawable)
{
  return drawable_create(original->image, new_drawable);
}

void drawable_destroy(drawable_t *d)
{
  if (d)
  {
    if (d->methods.reset)
      d->methods.reset(d);

    free(d);
  }
}

void drawable_reset(drawable_t *d)
{
  if (d->methods.reset)
    d->methods.reset(d);

  d->flags = 0;

  set_methods(d);
}
