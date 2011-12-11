/* --------------------------------------------------------------------------
 *    Name: set-width.c
 * Purpose: Vector - flexible array
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/bitwise.h"
#include "appengine/datastruct/array.h"

#include "appengine/datastruct/vector.h"

#include "impl.h"

error vector_set_width(vector_t *v, size_t width)
{
  void   *newbase;
  size_t  currsz;
  size_t  newsz;

  if (width == 0)
    return error_BAD_ARG;

  if (width == v->width)
    return error_OK;

  currsz = v->allocated * v->width;
  newsz  = v->used * width;

  /* Avoid calling realloc for the same size block. */
  if (currsz != newsz)
  {
    newbase = realloc(v->base, newsz);
    if (newbase == NULL)
      return error_OOM;
  }
  else
  {
    newbase = v->base;
  }

  if (width > v->width)
    array_stretch(newbase, v->used, v->width, width, 0);
  else
    array_squeeze(newbase, v->used, v->width, width);

  v->width     = width;
  /* v->used remains the same */
  v->allocated = v->used;
  v->base      = newbase;

  return error_OK;
}
