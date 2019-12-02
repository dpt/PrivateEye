/* --------------------------------------------------------------------------
 *    Name: stream-packbitscomp.c
 * Purpose: PackBits compression
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/io/stream.h"

#include "appengine/io/stream-packbits.h"

#define DBUG(args)

/* The literal-run-literal merging can only happen within the current buffer
 * (the state is reset to Initial when resumed), so if a smaller buffer is
 * used then less merging can happen. Once the buffer is exceeded then we
 * yield and those bytes are never seen again. */

enum { Initial, Literal, Run, LiteralRun };

typedef struct stream_packbitscomp
{
  stream         base;
  stream        *input;
  int            state;
  unsigned char *lastliteral;

  osbool         resume;
  int            n;
  unsigned char  first;

  int            bufsz;
  unsigned char  buffer[UNKNOWN];
}
stream_packbitscomp;

static error stream_packbitscomp_op(stream *s, stream_opcode op, void *arg)
{
  NOT_USED(s);
  NOT_USED(op);
  NOT_USED(arg);

  return error_STREAM_UNKNOWN_OP;
}

static void stream_packbitscomp_reset(stream_packbitscomp *c)
{
  c->state       = Initial;
  c->lastliteral = NULL;
  c->resume      = FALSE;
}

static int stream_packbitscomp_get(stream *s)
{
  stream_packbitscomp *sm = (stream_packbitscomp *) s;
  unsigned char       *p;
  unsigned char       *end;
  int                  n;
  int                  first;

  /* are we only called when buffer empty? */
  assert(sm->base.buf == sm->base.end);

  p   = sm->buffer;
  end = sm->buffer + sm->bufsz;

  if (sm->resume)
  {
    /* restore state */
    sm->state       = Initial;
    sm->lastliteral = NULL;
    sm->resume      = FALSE;
    n               = sm->n;
    first           = sm->first;
    goto again;
  }

  for (;;)
  {
    int second;

    /* find the longest string of identical input bytes */

    n = 1;
    first = stream_getc(sm->input);
    if (first == EOF)
      break;

    second = stream_getc(sm->input);
    while (second == first) /* terminates if EOF */
    {
      n++;
      second = stream_getc(sm->input);
    }
    /* if we didn't hit EOF, we will have read one byte too many, so put one
     * back */
    if (second != EOF)
      stream_ungetc(sm->input);

    DBUG(("stream_packbitscomp_get: n=%d\n", n));

again: /* more of the current run left to pack */

    /* we assume here that we need two spare bytes to continue (which is not
     * always true) */
    if (p + 2 > end)
    {
      /* save state */
      sm->resume = TRUE;
      sm->n      = n;
      sm->first  = first;
      break;
    }

    switch (sm->state)
    {
    case Initial: /* Initial state: Set state to 'Run' or 'Literal'. */
       DBUG(("stream_packbitscomp_get: Initial"));
       if (n > 1)
       {
         DBUG((" -> Run of %d\n", MIN(n, 128)));
         sm->state = Run;

         /* Clamp run lengths to a maximum of 128. Technically they could go
          * up to 129, but that would generate a -128 output which is
          * specified to be ignored by the PackBits spec. */

         *p++ = -MIN(n, 128) + 1;
         *p++ = first;
         n -= 128;

         if (n > 0)
           goto again;
       }
       else
       {
         DBUG((" -> Literal\n"));
         sm->state = Literal;

         sm->lastliteral = p;
         *p++ = 0; /* 1 repetition */
         *p++ = first;
       }
       break;

     case Literal: /* Last object was a literal. */
       DBUG(("stream_packbitscomp_get: Literal"));
       if (n > 1)
       {
         DBUG((" -> Run of %d\n", MIN(n, 128)));
         sm->state = LiteralRun;

         *p++ = -MIN(n, 128) + 1;
         *p++ = first;
         n -= 128;

         if (n > 0)
           goto again;
       }
       else
       {
         DBUG((" -> Literal\n"));

         assert(sm->lastliteral);

         *p++ = first;

         /* extend the previous literal */
         DBUG((" extending previous\n"));
         if (++(*sm->lastliteral) == 127)
         {
           DBUG((" -> Initial\n"));
           sm->state = Initial;
         }
       }
       break;

     case Run: /* Last object was a run. */
       DBUG(("stream_packbitscomp_get: Run"));
       if (n > 1)
       {
         DBUG((" -> Run\n"));

         *p++ = -MIN(n, 128) + 1;
         *p++ = first;
         n -= 128;

         if (n > 0)
             goto again;
       }
       else
       {
         DBUG((" -> Literal\n"));
         sm->state = Literal;

         sm->lastliteral = p;
         *p++ = 0; /* 1 repetition */
         *p++ = first;
       }
       break;

     case LiteralRun: /* last object was a run, preceded by a literal */
       {
       int ll;

       DBUG(("stream_packbitscomp_get: LiteralRun"));

       assert(sm->lastliteral);

       ll = *sm->lastliteral;

       /* Check to see if previous run should be converted to a literal, in
        * which case we convert literal-run-literal to a single literal. */
       if (n == 1 && p[-2] == (unsigned char) -1 && ll < 126)
       {
           DBUG((" merge literal-run-literal\n"));
           ll += 2;
           sm->state = (ll == 127) ? Initial : Literal;
           *sm->lastliteral = ll;
           p[-2] = p[-1];
       }
       else
       {
           DBUG((" -> Run\n"));
           sm->state = Run;
       }
       goto again;

       }
    }
  }

  if (p == sm->buffer)
  {
    DBUG(("stream_packbitscomp_get: no bytes generated in decomp\n"));
    return EOF; /* EOF at start */
  }

  sm->base.buf = sm->buffer;
  sm->base.end = p;

  return *sm->base.buf++;
}

error stream_packbitscomp_create(stream *input, int bufsz, stream **s)
{
  stream_packbitscomp *sp;

  if (bufsz <= 0)
    bufsz = 128;

  if (bufsz < 2)
    return error_BAD_ARG; /* The buffer size needs to be at least 2 bytes
                           * long. */

  assert(input);

  sp = malloc(offsetof(stream_packbitscomp, buffer) + bufsz);
  if (!sp)
    return error_OOM;

  sp->base.buf     =
  sp->base.end     = sp->buffer; /* force a fill on first use */

  sp->base.last    = error_OK;

  sp->base.op      = stream_packbitscomp_op;
  sp->base.seek    = NULL; /* can't seek */
  sp->base.get     = stream_packbitscomp_get;
  sp->base.length  = NULL; /* unknown length */
  sp->base.destroy = NULL;

  sp->input = input;
  sp->bufsz = bufsz;

  stream_packbitscomp_reset(sp);

  *s = &sp->base;

  return error_OK;
}
