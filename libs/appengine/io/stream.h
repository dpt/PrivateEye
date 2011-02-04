/* --------------------------------------------------------------------------
 *    Name: stream.h
 * Purpose: Stream interface
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_STREAM_H
#define APPENGINE_STREAM_H

#include "appengine/base/errors.h"

typedef enum stream__opcode
{
  stream__IN_MEMORY /* query whether stream is contained in memory,
                     * returns an int */
}
stream__opcode;

#define T stream

typedef struct T T;

struct T
{
  const unsigned char *buf; /* current buffer pointer */
  const unsigned char *end; /* end of buffer pointer (points to char after buffer end) */

  error  last; /* last error. set when we return EOF? */

  error (*op)(T *s, stream__opcode opcode, void *arg);
  error (*seek)(T *s, int pos);
  int   (*get)(T *s); /* uses standard EOF constant from stdio */
  int   (*length)(T *s);
  void  (*destroy)(T *doomed);
};

error stream__op(T *s, stream__opcode opcode, void *arg);
error stream__seek(T *s, int pos);
int   stream__fill(T *s);
int   stream__length(T *s);
void  stream__destroy(T *doomed);

/* Get a byte from a stream. */
#define stream__getc(s) (((s)->buf != (s)->end) ? *(s)->buf++ : (s)->get(s))

/* Put back the last byte gotten. */
#define stream__ungetc(s) --(s)->buf;

/* Returns the number of bytes remaining in the current buffer. */
#define stream__remaining(s) ((s)->end - (s)->buf)

/* Returns the number of bytes remaining in the current buffer.
 * Will attempt to fill the buffer if it's found to be empty. */
#define stream__remaining_and_fill(s) (stream__remaining(s) != 0 ? \
                                       stream__remaining(s) : \
                                       stream__fill(s))

#undef T

#endif /* APPENGINE_STREAM_H */
