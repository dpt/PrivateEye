/* --------------------------------------------------------------------------
 *    Name: stream.c
 * Purpose: Stream interface
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"

#include "appengine/io/stream.h"

error stream__seek(stream *s, int pos)
{
  if (!s->seek)
    return error_STREAM_CANT_SEEK;

  return s->seek(s, pos);
}

int stream__length(stream *s)
{
  if (!s->length)
    return -1;

  return s->length(s);
}

error stream__op(stream *s, stream__opcode opcode, void *arg)
{
  if (!s->op)
    return error_STREAM_UNKNOWN_OP;

  return s->op(s, opcode, arg);
}

void stream__destroy(stream *doomed)
{
  if (!doomed)
    return;

  if (doomed->destroy)
    doomed->destroy(doomed);

  free(doomed);
}

int stream__fill(stream *s)
{
  int c;

  c = s->get(s);
  if (c == EOF)
    return 0;

  s->buf--;

  return stream__remaining(s);
}

