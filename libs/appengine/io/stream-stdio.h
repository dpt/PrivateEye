/* --------------------------------------------------------------------------
 *    Name: stream-stdio.h
 * Purpose: C standard IO stream implementation
 * Version: $Id: stream-stdio.h,v 1.3 2010-01-29 14:48:17 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_STDIO_H
#define APPENGINE_STREAM_STDIO_H

#include <stdio.h>

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

/* use 0 for a sensible default buffer size */

error stream_file__create(FILE *f, int bufsz, stream **s);

#endif /* APPENGINE_STREAM_STDIO_H */
