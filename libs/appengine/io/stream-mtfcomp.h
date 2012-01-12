/* --------------------------------------------------------------------------
 *    Name: stream-mtfcomp.h
 * Purpose: "Move to front" adaptive compression stream implementation
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_MTFCOMP_H
#define APPENGINE_STREAM_MTFCOMP_H

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

error stream_mtfcomp_create(stream *input, int bufsz, stream **s);
error stream_mtfdecomp_create(stream *input, int bufsz, stream **s);

#endif /* APPENGINE_STREAM_MTFCOMP_H */
