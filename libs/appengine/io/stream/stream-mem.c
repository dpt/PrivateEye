/* --------------------------------------------------------------------------
 *    Name: stream-mem.c
 * Purpose: Memory block IO stream implementation
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

typedef struct stream_mem
{
  stream               base;

  const unsigned char *block;
  size_t               length;
}
stream_mem;

static error stream_mem_seek(stream *s, int pos)
{
  stream_mem *sf = (stream_mem *) s;

  if (pos > sf->length) /* allow seeks equal to file length (to EOF) */
    return error_STREAM_BAD_SEEK;

  sf->base.buf = sf->block + pos;

  return error_OK;
}

static int stream_mem_get(stream *s)
{
  stream_mem *sf = (stream_mem *) s;

  NOT_USED(sf); /* only used in debug builds */

  /* are we only called when buffer empty? */
  assert(sf->base.buf == sf->base.end);

  return EOF;
}

static int stream_mem_length(stream *s)
{
  stream_mem *sf = (stream_mem *) s;

  return sf->length;
}

error stream_mem_create(const unsigned char *block,
                        size_t               length,
                        stream             **s)
{
  stream_mem *sf;

  assert(block);

  sf = malloc(sizeof(*sf));
  if (!sf)
    return error_OOM;

  sf->base.buf     = block;
  sf->base.end     = block + length;

  sf->base.last    = error_OK;

  sf->base.op      = NULL;
  sf->base.seek    = stream_mem_seek;
  sf->base.get     = stream_mem_get;
  sf->base.length  = stream_mem_length;
  sf->base.destroy = NULL;

  sf->block  = block;
  sf->length = length;

  *s = &sf->base;

  return error_OK;
}
