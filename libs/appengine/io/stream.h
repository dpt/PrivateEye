/* --------------------------------------------------------------------------
 *    Name: stream.h
 * Purpose: Stream interface
 * ----------------------------------------------------------------------- */

/**
 * \file Stream (interface).
 *
 * Data source.
 */

#ifndef APPENGINE_STREAM_H
#define APPENGINE_STREAM_H

#include "appengine/base/errors.h"

typedef enum stream_opcode
{
  stream_IN_MEMORY /* query whether stream is contained in memory,
                     * returns an int */
}
stream_opcode;

#define T stream

typedef struct T T;

struct T
{
  const unsigned char *buf; /* current buffer pointer */
  const unsigned char *end; /* end of buffer pointer (points to char after buffer end) */

  error  last; /* last error. set when we return EOF? */

  error (*op)(T *s, stream_opcode opcode, void *arg);
  error (*seek)(T *s, int pos);
  int   (*get)(T *s); /* uses standard EOF constant from stdio */
  int   (*length)(T *s);
  void  (*destroy)(T *doomed);
};

error stream_op(T *s, stream_opcode opcode, void *arg);
error stream_seek(T *s, int pos);
int   stream_fill(T *s);
int   stream_length(T *s);
void  stream_destroy(T *doomed);

/* Get a byte from a stream. */
#define stream_getc(s) (((s)->buf != (s)->end) ? *(s)->buf++ : (s)->get(s))

/* Put back the last byte gotten. */
#define stream_ungetc(s) --(s)->buf;

/* Returns the number of bytes remaining in the current buffer. */
#define stream_remaining(s) ((s)->end - (s)->buf)

/* Returns the number of bytes remaining in the current buffer.
 * Will attempt to fill the buffer if it's found to be empty. */
#define stream_remaining_and_fill(s) (stream_remaining(s) != 0 ? \
                                       stream_remaining(s) : \
                                       stream_fill(s))

#undef T

#endif /* APPENGINE_STREAM_H */
