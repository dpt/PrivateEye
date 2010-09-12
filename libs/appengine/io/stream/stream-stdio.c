/* --------------------------------------------------------------------------
 *    Name: stream-stdio.c
 * Purpose: C standard IO stream implementation
 * Version: $Id: stream-stdio.c,v 1.4 2010-01-29 14:48:17 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

#include "appengine/io/stream-stdio.h"

typedef struct stream_file
{
  stream         base;
  FILE          *file;
  long int       length;

  int            bufsz;
  unsigned char  buffer[UNKNOWN];
}
stream_file;

static error stream_file__seek(stream *s, int pos)
{
  stream_file *sf = (stream_file *) s;

  fseek(sf->file, pos, SEEK_SET);

  sf->base.buf = sf->base.end; /* force a re-fill */

  return error_OK;
}

static int stream_file__get(stream *s)
{
  stream_file *sf = (stream_file *) s;
  int         read;

  /* are we only called when buffer empty? */
  assert(sf->base.buf == sf->base.end);

  read = fread(sf->buffer, 1, sf->bufsz, sf->file);
  if (read == 0 && feof(sf->file))
    return EOF;

  sf->base.buf = sf->buffer;
  sf->base.end = sf->buffer + read;

  return *sf->base.buf++;
}

static int stream_file__length(stream *s)
{
  stream_file *sf = (stream_file *) s;

  if (sf->length < 0)
  {
    long int pos;

    /* cache the file's length */

    pos = ftell(sf->file);
    fseek(sf->file, 0, SEEK_END);
    sf->length = ftell(sf->file);
    fseek(sf->file, pos, SEEK_SET);
  }

  return (int) sf->length;
}

static void stream_file__destroy(stream *doomed)
{
  stream_file *sf = (stream_file *) doomed;

  fclose(sf->file);
}

error stream_file__create(FILE *f, int bufsz, stream **s)
{
  stream_file *sf;

  if (bufsz <= 0)
    bufsz = 128;

  assert(f);

  sf = malloc(offsetof(stream_file, buffer) + bufsz);
  if (!sf)
      return error_OOM;

  sf->base.buf     =
  sf->base.end     = sf->buffer; /* force a fill on first use */

  sf->base.last    = error_OK;

  sf->base.op      = NULL;
  sf->base.seek    = stream_file__seek;
  sf->base.get     = stream_file__get;
  sf->base.length  = stream_file__length;
  sf->base.destroy = stream_file__destroy;

  sf->file   = f;
  sf->length = -1;
  sf->bufsz  = bufsz;

  *s = &sf->base;

  return error_OK;
}
