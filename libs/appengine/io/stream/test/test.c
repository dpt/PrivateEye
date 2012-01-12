
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/strings.h"

#include "appengine/io/stream.h"
#include "appengine/io/stream-stdio.h"
#include "appengine/io/stream-mtfcomp.h"
#include "appengine/io/stream-packbits.h"
#include "appengine/io/stream-mem.h"

#define BUFSZ 0 /* use the default buffer size in each case */

#define USEFILE 0

typedef enum Index
{
  Source,
  MTFComp,
  MTFDecomp,
  PackBitsComp,
  PackBitsDecomp,
  MaxStreams,
}
Index;

#if !USEFILE
static const unsigned char input[] =
"abbcccddddeeeeeffffff\n"
"a bb ccc dddd eeeee ffffff\n"
"We all love aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaardvarks.\n"
"and iiiiii\n"
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
#endif

int stream_test(void)
{
  error                err = error_OK;
#if USEFILE
  FILE                *in;
  FILE                *out;
#endif
  stream              *stream[MaxStreams];
  int                  c;
  int                  i;
#if !USEFILE
  const unsigned char *p;
#endif

#if USEFILE
  in = fopen("stream-test-input", "rb");
  if (in == NULL)
    goto Failure;

  out = fopen("stream-test-output", "wb");
  if (out == NULL)
    goto Failure;
#endif

#if USEFILE
  err = stream_stdio_create(in, BUFSZ, &stream[Source]);
#else
  err = stream_mem_create(input, NELEMS(input), &stream[Source]);
#endif
  if (err)
    goto Failure;

  /* source -> mtfcomp -> mtfdecomp -> packbitscomp -> packbitsdecomp */

  err = stream_mtfcomp_create(stream[Source], BUFSZ, &stream[MTFComp]);
  if (err)
    goto Failure;

  err = stream_mtfdecomp_create(stream[MTFComp], BUFSZ, &stream[MTFDecomp]);
  if (err)
    goto Failure;

  err = stream_packbitscomp_create(stream[MTFDecomp], BUFSZ, &stream[PackBitsComp]);
  if (err)
    goto Failure;

  err = stream_packbitsdecomp_create(stream[PackBitsComp], BUFSZ, &stream[PackBitsDecomp]);
  if (err)
    goto Failure;

#if USEFILE
  for (;;)
  {
    c = stream_getc(stream[PackBitsDecomp]);
    if (c == EOF)
      break;

    fputc(c, out);
  }
#else
  for (p = input; ; p++)
  {
    c = stream_getc(stream[PackBitsDecomp]);
    if (c == EOF)
      break;

    if (c != *p)
      printf("difference at %d\n", p - input);
  }

  printf("%d bytes processed.\n", p - input);
#endif

  for (i = 0; i < MaxStreams; i++)
    stream_destroy(stream[i]);

#if USEFILE
  fclose(out);
  /* fclose(in) is handled by the FILE stream's destruction */
#endif

  return 0;


Failure:

  printf("\n\n*** Error %lx\n", err);

  return 1;
}
