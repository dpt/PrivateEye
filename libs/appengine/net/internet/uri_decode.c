
#include "kernel.h"
#include "swis.h"

#include <stdlib.h>
#include <string.h>

#include "appengine/base/appengine.h"
#include "appengine/base/strings.h"

#include "appengine/net/internet.h"

/*int main (void)
{
  char uri[] = "http://bilbo:baggins@www.hobbit.com:/path?query";
  char scheme[6], creds[32], host[64], path[256];
  int port;

  uri_decode(uri, scheme, creds, host, &port, path);

  printf("%s\n%s\n%s\n%i\n%s\n", scheme, creds, host, port, path);
}*/

int uri_decode(const char *uri,
               char       *scheme,
               char       *creds,
               char       *host,
               int        *port,
               char       *path)
{
  const char *p;
  char       *q, *r;

  p = uri;

  /* Scheme */
  q = scheme;
  while (*p >= 'a' && *p <= 'z')
    *q++ = *p++;
  *q++ = '\0';

  if (*p++ != ':' || *p++ != '/' || *p++ != '/')
    return -1; /* invalid */

  /* Credentials, Host and Port */
  q = host;
  while (*p > ' ' && *p != '/')
    *q++ = *p++;
  *q++ = '\0';
  /* are there credentials in the host string? */
  r = strchr(host, '@');
  if (r != NULL)
  {
    *r = '\0';
    str_cpy(creds, host);
    str_cpy(host, r + 1);
  }
  else
  {
    *creds = '\0';
  }
  /* Port specifier */
  r = strchr(host, ':');
  if (r != NULL)
  {
    *r++ = '\0';
    if (*r >= '0' && *r <= '9')
      *port = atoi(r);
    else
      *port = 80;
  }
  else
  {
    *port = 80;
  }

  /* Path specifier */
  if (*p > ' ')
  {
    q = path;
    while (*p > ' ')
      *q++ = *p++;
    *q++ = '\0';
  }
  else
  {
    /* End of string, without path specified, assume default of '/'. */
    path = "/";
  }

  return 0; /* valid */
}
