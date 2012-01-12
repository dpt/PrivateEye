/* --------------------------------------------------------------------------
 *    Name: stream-packbits.h
 * Purpose: PackBits compression
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_PACKBITS_H
#define APPENGINE_STREAM_PACKBITS_H

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

/* use 0 for a sensible default buffer size */

error stream_packbitscomp_create(stream *input, int bufsz, stream **s);
error stream_packbitsdecomp_create(stream *input, int bufsz, stream **s);

#endif /* APPENGINE_STREAM_PACKBITS_H */
