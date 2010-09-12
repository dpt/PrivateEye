/* $Id: file.c,v 1.1 2010-01-09 22:08:10 dpt Exp $ */

#include <stdio.h>
#include <stddef.h>

#include "appengine/base/appengine.h"
#include "appengine/io/basic.h"

static FILE *data = NULL;

/*
 * basic_data_open:
 */

int basic_data_open(char *filename)
{
  data = fopen(filename, "rb");
  if (data == NULL)
    return 1;	/* failure */

  return 0;	/* success */
}

/*
 * basic_data_close:
 */

void basic_data_close(void)
{
  if (data == NULL)
    return;

  fclose(data);
  data = NULL;
}

/*
 * basic_data_read:
 */

int basic_data_read(void *buffer_v, int buffer_size)
{
  char *buffer;
  int c, d;

  if (data == NULL)
    return -1;	/* no data file open */

  buffer = (char *) buffer_v;

  c = fgetc(data);
  switch (c)
  {
    case 0:	/* string (length specified as first byte) */
      d = fgetc(data);
      if (buffer_size <= d)
        /* buffer insufficient (even with accounting for terminator byte) */
        return -1;
      buffer[d] = '\0';
      while (d)
        buffer[--d] = fgetc(data);
      break;

    case 64:	/* integer (four bytes, big endian) */
      d = 4;
      while (d)
        buffer[--d] = fgetc(data);
      break;

    case 128:	/* real (four bytes) */
      break;
  }

  return c;
}
