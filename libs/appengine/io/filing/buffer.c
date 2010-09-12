/* $Id: buffer.c,v 1.1 2010-01-09 22:21:11 dpt Exp $ */

#include "kernel.h"
#include "swis.h"

#include <string.h>

#include "appengine/base/appengine.h"
#include "appengine/base/oserror.h"
#include "appengine/base/heap.h"

#include "appengine/io/filing.h"

static int buffer_size;
static char *buffer = NULL;
static char *buffer_ptr;
static char *buffer_end;
static char *h; /* base of heap */
static int handle;

int buffer_start(const char *file_name, size_t suggested_buffer_size)
{
  if (buffer != NULL)
    return -1; /* failure - already caching */

  if (EC(_swix(OS_Find, _INR(0,1)|_OUT(0), 0x4f, file_name, &handle)) != NULL)
    return -1; /* failure - file not found */

  buffer_size = suggested_buffer_size;

  h = heap_create("AppEngine filing buffer", buffer_size + 64 /* allow some slack for heap data */);
  if (h == NULL)
    return -1; /* failure - failed to create heap */

  buffer = heap_claim(h, buffer_size);
  if (buffer == NULL)
    return -1; /* failure - failed to claim buffer */

  /* first read will cause a buffer refill */
  buffer_ptr = buffer_end = buffer + buffer_size;

  return 0; /* success */
}

void buffer_stop(void)
{
  if (buffer == NULL)
    return; /* already stopped */

  heap_release(h, buffer);
  buffer = NULL;

  heap_delete(h);

  EC(_swix(OS_Find, _INR(0,1), 0x00, handle));
}

static int buffer_refill(void)
{
  int read;

  if (buffer == NULL)
    return -1; /* failure - not caching */

  if (EC(_swix(OS_GBPB, _INR(0,3)|_OUT(3), 4, handle, buffer, buffer_size, &read /* unread */ )) != NULL)
    return -1;

  /* reading 0 bytes is ok */

  read = buffer_size - read;

  buffer_ptr = buffer;
  buffer_end = buffer + read;

  return read;
}

int buffer_getbyte(void)
{
  if (buffer_ptr == buffer_end)
    if (buffer_refill() <= 0)
      return -1; /* failure or eof */

  return *buffer_ptr++;
}

int buffer_getblock(char *to, size_t block_size)
{
  int copy;
  int copied;
  int read;

  copied = 0;
  while (block_size)
  {
    if (buffer_ptr == buffer_end)
    {
      read = buffer_refill();
      if (read < 0) /* failure */
        return -1;
      else if (read == 0) /* eof */
        return copied;
    }
    if (block_size > buffer_end - buffer_ptr)   /* bytes in buffer */
      copy = buffer_end - buffer_ptr;
    else
      copy = block_size;
    memcpy((char *) to, (char *) buffer_ptr, copy);
    copied += copy;
    to += copy;
    buffer_ptr += copy;
    block_size -= copy;
  }

  return copied;
}
