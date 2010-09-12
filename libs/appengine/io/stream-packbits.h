/* --------------------------------------------------------------------------
 *    Name: stream-packbits.h
 * Purpose: PackBits compression
 * Version: $Id: stream-packbits.h,v 1.3 2010-01-29 14:48:17 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_PACKBITS_H
#define APPENGINE_STREAM_PACKBITS_H

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

/* use 0 for a sensible default buffer size */

error stream_packbitscomp__create(stream *input, int bufsz, stream **s);
error stream_packbitsdecomp__create(stream *input, int bufsz, stream **s);

#endif /* APPENGINE_STREAM_PACKBITS_H */
