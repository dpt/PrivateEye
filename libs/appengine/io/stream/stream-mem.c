/* --------------------------------------------------------------------------
 *    Name: stream-mem.c
 * Purpose: Memory block IO stream implementation
 * Version: $Id: stream-mem.c,v 1.2 2010-01-29 14:48:17 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

#include "appengine/io/stream-mem.h"

typedef struct stream__mem
{
  stream               base;

  const unsigned char *block;
  size_t               length;
}
stream__mem;

static error stream_mem__seek(stream *s, int pos)
{
  stream__mem *sf = (stream__mem *) s;

  if (pos > sf->length) /* allow seeks equal to file length (to EOF) */
    return error_STREAM_BAD_SEEK;

  sf->base.buf = sf->block + pos;

  return error_OK;
}

static int stream_mem__get(stream *s)
{
  stream__mem *sf = (stream__mem *) s;

  NOT_USED(sf); /* only used in debug builds */

  /* are we only called when buffer empty? */
  assert(sf->base.buf == sf->base.end);

  return EOF;
}

static int stream_mem__length(stream *s)
{
  stream__mem *sf = (stream__mem *) s;

  return sf->length;
}

error stream_mem__create(const unsigned char *block,
                         size_t               length,
                         stream             **s)
{
  stream__mem *sf;

  assert(block);

  sf = malloc(sizeof(*sf));
  if (!sf)
      return error_OOM;

  sf->base.buf     = block;
  sf->base.end     = block + length;

  sf->base.last    = error_OK;

  sf->base.op      = NULL;
  sf->base.seek    = stream_mem__seek;
  sf->base.get     = stream_mem__get;
  sf->base.length  = stream_mem__length;
  sf->base.destroy = NULL;

  sf->block  = block;
  sf->length = length;

  *s = &sf->base;

  return error_OK;
}
