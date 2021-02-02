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

#define BUFSZ 65536

result_t md5_from_file(const char    *file_name,
                    unsigned char  digest[md5_DIGESTSZ])
{
  result_t               err;
  unsigned char      *buffer;
  FILE               *in = NULL;
  long                len;
  struct md5_context  md5;
  int                 c;

  hourglass_on();

  buffer = malloc(BUFSZ);
  if (buffer == NULL)
  {
    err = result_OOM;
    goto failure;
  }

  in = fopen(file_name, "rb");
  if (in == NULL)
  {
    err = result_FILE_OPEN_FAILED;
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

  md5_final(digest, &md5);

  fclose(in);

  free(buffer);

  hourglass_off();

  return result_OK;


failure:

  fclose(in);

  free(buffer);

  hourglass_off();

  return err;
}
