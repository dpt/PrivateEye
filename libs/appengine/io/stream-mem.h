/* --------------------------------------------------------------------------
 *    Name: stream-mem.h
 * Purpose: Memory block IO stream implementation
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_MEM_H
#define APPENGINE_STREAM_MEM_H

#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

error stream_mem__create(const unsigned char *block,
                         size_t               length,
                         stream             **s);

#endif /* APPENGINE_STREAM_MEM_H */
