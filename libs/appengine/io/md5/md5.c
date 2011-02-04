/* --------------------------------------------------------------------------
 *    Name: md5.c
 * Purpose: MD5 digest
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "md5/md5.h"

#include "oslib/hourglass.h"
#include "oslib/osfile.h"
#include "oslib/osfscontrol.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"

#include "appengine/io/md5.h"

static void digest_to_string(char *s, const unsigned char *digest)
{
  char *p;
  int   i;

  for (p = s, i = 0; i < 16; i++, p += 2)
    sprintf(p, "%02X", *digest++);
}

#define BUFSZ 65536

error md5__from_file(const char *file_name, char *digest)
{
  error               err;
  unsigned char      *buffer;
  FILE               *in = NULL;
  long                len;
  struct md5_context  md5;
  int                 c;
  unsigned char       binarydigest[16];

  hourglass_on();

  buffer = malloc(BUFSZ);
  if (buffer == NULL)
  {
    err = error_OOM;
    goto failure;
  }

  in = fopen(file_name, "rb");
  if (in == NULL)
  {
    err = error_FILE_OPEN_FAILED;
    goto failure;
  }

  fseek(in, 0, SEEK_END);
  len = ftell(in);
  fseek(in, 0, SEEK_SET);

  md5_init(&md5);

  while ((c = (int) fread(buffer, 1, BUFSZ, in)) > 0)
  {
    md5_update(&md5, buffer, c);
    hourglass_percentage((int) (100 * ftell(in) / len));
  }

  md5_final(binarydigest, &md5);

  fclose(in);

  free(buffer);

  digest_to_string(digest, binarydigest);

  hourglass_off();

  return error_OK;


failure:

  fclose(in);

  free(buffer);

  hourglass_off();

  return err;
}
