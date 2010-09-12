/* --------------------------------------------------------------------------
 *    Name: stream-mem.h
 * Purpose: Memory block IO stream implementation
 * Version: $Id: stream-mem.h,v 1.2 2010-01-29 14:48:17 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_MEM_H
#define APPENGINE_STREAM_MEM_H

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

error stream_mem__create(const unsigned char *block,
                         size_t               length,
                         stream             **s);

#endif /* APPENGINE_STREAM_MEM_H */
