/* --------------------------------------------------------------------------
 *    Name: stream-mtfcomp.h
 * Purpose: "Move to front" adaptive compression stream implementation
 * Version: $Id: stream-mtfcomp.h,v 1.3 2010-01-29 14:48:17 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_MTFCOMP_H
#define APPENGINE_STREAM_MTFCOMP_H

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

error stream_mtfcomp__create(stream *input, int bufsz, stream **s);
error stream_mtfdecomp__create(stream *input, int bufsz, stream **s);

#endif /* APPENGINE_STREAM_MTFCOMP_H */
